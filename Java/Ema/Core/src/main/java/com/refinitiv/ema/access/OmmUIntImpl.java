///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.math.BigInteger;


class OmmUIntImpl extends DataImpl implements OmmUInt
{
	private com.refinitiv.eta.codec.UInt _rsslUInt = com.refinitiv.eta.codec.CodecFactory.createUInt();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.UINT;
	}

	@Override
	public long longValue()
	{
		return _rsslUInt.toLong();
	}

	@Override
	public BigInteger bigIntegerValue()
	{
		return _rsslUInt.toBigInteger();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslUInt.toString();
	}
	
	@Override
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS == _rsslUInt.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslUInt.clear();
		}
	}
}