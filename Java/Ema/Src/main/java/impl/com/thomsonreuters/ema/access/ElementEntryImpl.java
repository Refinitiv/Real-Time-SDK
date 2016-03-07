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
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.upa.codec.CodecFactory;

class ElementEntryImpl extends EntryImpl implements ElementEntry
{
	private com.thomsonreuters.upa.codec.ElementEntry	_rsslElementEntry;
	
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

		if (_load.dataType() <= DataTypes.ARRAY)
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
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry refreshMsg(String name, RefreshMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry statusMsg(String name, StatusMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry updateMsg(String name, UpdateMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry postMsg(String name, PostMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry ackMsg(String name, AckMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry genericMsg(String name, GenericMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry fieldList(String name, FieldList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry elementList(String name, ElementList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry map(String name, Map value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry vector(String name, Vector value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry series(String name, Series value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry filterList(String name, FilterList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry opaque(String name, OmmOpaque value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry xml(String name, OmmXml value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry ansiPage(String name, OmmAnsiPage value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry intValue(String name, long value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry uintValue(String name, long value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry uintValue(String name, BigInteger value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry real(String name, long mantissa, int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry realFromDouble(String name, double value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry realFromDouble(String name, double value, int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry floatValue(String name, float value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry doubleValue(String name, double value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry date(String name, int year, int month, int day)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry time(String name, int hour, int minute)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry time(String name, int hour, int minute, int second)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond, int microsecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond, int microsecond,
			int nanosecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, int second)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, int second,
			int millisecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, int second,
			int millisecond, int microsecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, int second,
			int millisecond, int microsecond, int nanosecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry qos(String name, int timeliness)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry qos(String name, int timeliness, int rate)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry state(String name, int streamState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry state(String name, int streamState, int dataState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry state(String name, int streamState, int dataState, int statusCode)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry state(String name, int streamState, int dataState, int statusCode, String statusText)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry enumValue(String name, int value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry buffer(String name, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry ascii(String name, String value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry utf8(String name, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry utf8(String name, String value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry rmtes(String name, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry array(String name, OmmArray value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeInt(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeUInt(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeReal(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeFloat(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeDouble(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeDate(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeTime(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeDateTime(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeQos(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeState(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeEnum(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeBuffer(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeAscii(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeUtf8(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementEntry codeRmtes(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}
}