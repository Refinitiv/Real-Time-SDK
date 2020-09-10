///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;


class OmmTimeImpl extends DataImpl implements OmmTime
{
	com.rtsdk.eta.codec.Time _rsslTime = com.rtsdk.eta.codec.CodecFactory.createTime();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.TIME;
	}

	@Override
	public int hour()
	{
		return _rsslTime.hour();
	}

	@Override
	public int minute()
	{
		return _rsslTime.minute();
	}

	@Override
	public int second()
	{
		return _rsslTime.second();
	}

	@Override
	public int millisecond()
	{
		return _rsslTime.millisecond();
	}

	@Override
	public int microsecond()
	{
		return _rsslTime.microsecond();
	}

	@Override
	public int nanosecond()
	{
		return _rsslTime.nanosecond();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslTime.toString();
	}

	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, com.rtsdk.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.rtsdk.eta.codec.CodecReturnCodes.SUCCESS == _rsslTime.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslTime.blank();
		}
	}
}