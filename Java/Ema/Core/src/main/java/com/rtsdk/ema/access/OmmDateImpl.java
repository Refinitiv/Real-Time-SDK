///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

class OmmDateImpl extends DataImpl implements OmmDate
{
	 com.rtsdk.eta.codec.Date _rsslDate = com.rtsdk.eta.codec.CodecFactory.createDate();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.DATE;
	}

	@Override
	public int year()
	{
		return _rsslDate.year();
	}

	@Override
	public int month()
	{
		return _rsslDate.month();
	}

	@Override
	public int day()
	{
		return _rsslDate.day();
	}
	
	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslDate.toString();
	}

	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, com.rtsdk.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.rtsdk.eta.codec.CodecReturnCodes.SUCCESS == _rsslDate.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslDate.blank();
		}
	}
}