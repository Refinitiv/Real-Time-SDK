///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmTime;
import com.thomsonreuters.upa.codec.CodecFactory;

class OmmTimeImpl extends DataImpl implements OmmTime
{
	private com.thomsonreuters.upa.codec.Time _rsslTime = CodecFactory.createTime();
	
	@Override
	public int dataType()
	{
		return DataTypes.TIME;
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
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS == _rsslTime.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslTime.blank();
		}
	}
}