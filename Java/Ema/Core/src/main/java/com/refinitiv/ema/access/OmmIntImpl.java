///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


class OmmIntImpl extends DataImpl implements OmmInt
{
	private com.refinitiv.eta.codec.Int _rsslInt = com.refinitiv.eta.codec.CodecFactory.createInt();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.INT;
	}

	@Override
	public long intValue()
	{
		return _rsslInt.toLong();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslInt.toString();
	}

	@Override
	int decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS == _rsslInt.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslInt.clear();
		}

		return com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
	}
}