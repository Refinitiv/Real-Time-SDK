///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;


class OmmFloatImpl extends DataImpl implements OmmFloat
{
	private com.rtsdk.eta.codec.Float _rsslFloat = com.rtsdk.eta.codec.CodecFactory.createFloat();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.FLOAT;
	}

	@Override
	public float floatValue()
	{
		return _rsslFloat.toFloat();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslFloat.toString();
	}
	
	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, com.rtsdk.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.rtsdk.eta.codec.CodecReturnCodes.SUCCESS == _rsslFloat.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslFloat.blank();
		}
	}
}