///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;


class OmmDateTimeImpl extends DataImpl implements OmmDateTime
{
	com.rtsdk.eta.codec.DateTime _rsslDateTime = com.rtsdk.eta.codec.CodecFactory.createDateTime();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.DATETIME;
	}

	@Override
	public int year()
	{
		return _rsslDateTime.year();
	}

	@Override
	public int month()
	{
		return _rsslDateTime.month();
	}

	@Override
	public int day()
	{
		return _rsslDateTime.day();
	}

	@Override
	public int hour()
	{
		return _rsslDateTime.hour();
	}

	@Override
	public int minute()
	{
		return _rsslDateTime.minute();
	}

	@Override
	public int second()
	{
		return _rsslDateTime.second();
	}

	@Override
	public int millisecond()
	{
		return _rsslDateTime.millisecond();
	}

	@Override
	public int microsecond()
	{
		return _rsslDateTime.microsecond();
	}

	@Override
	public int nanosecond()
	{
		return _rsslDateTime.nanosecond();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslDateTime.toString();
	}

	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, com.rtsdk.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.rtsdk.eta.codec.CodecReturnCodes.SUCCESS == _rsslDateTime.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslDateTime.blank();
		}
	}
}