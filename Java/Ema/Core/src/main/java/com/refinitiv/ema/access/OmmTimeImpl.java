///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


class OmmTimeImpl extends DataImpl implements OmmTime
{
	com.refinitiv.eta.codec.Time _rsslTime = com.refinitiv.eta.codec.CodecFactory.createTime();
	
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
	int decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		int decodeRetValue = _rsslTime.decode(dIter);
		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS == decodeRetValue)
			_dataCode = DataCode.NO_CODE;
		else if (com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA == decodeRetValue)
			return com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslTime.blank();
		}

		return com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
	}
}