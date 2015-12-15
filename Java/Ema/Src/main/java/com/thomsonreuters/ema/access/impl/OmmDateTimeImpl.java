///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmDateTime;
import com.thomsonreuters.upa.codec.CodecFactory;

class OmmDateTimeImpl extends DataImpl implements OmmDateTime
{
	private com.thomsonreuters.upa.codec.DateTime _rsslDateTime = CodecFactory.createDateTime();
	
	@Override
	public int dataType()
	{
		return DataTypes.DATETIME;
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
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS == _rsslDateTime.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslDateTime.blank();
		}
	}
}