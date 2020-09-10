///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;


class OmmDoubleImpl extends DataImpl implements OmmDouble
{
	private com.rtsdk.eta.codec.Double _rsslDouble = com.rtsdk.eta.codec.CodecFactory.createDouble();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.DOUBLE;
	}

	@Override
	public double doubleValue()
	{
		return _rsslDouble.toDouble();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslDouble.toString();
	}
	
	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, com.rtsdk.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.rtsdk.eta.codec.CodecReturnCodes.SUCCESS == _rsslDouble.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslDouble.blank();
		}
	}
}