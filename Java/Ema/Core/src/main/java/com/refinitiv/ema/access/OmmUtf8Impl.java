///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.CodecReturnCodes;

class OmmUtf8Impl extends DataImpl implements OmmUtf8
{
	OmmUtf8Impl()
	{
		_rsslBuffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
	}

	@Override
	public int dataType()
	{
		return DataType.DataTypes.UTF8;
	}

	@Override
	public String string()
	{
		if (_rsslBuffer.length() == 0)
			return DataImpl.EMPTY_STRING;
		else
			return _rsslBuffer.toString();
	}
	
	@Override
	public ByteBuffer buffer()
	{
		return asHex();
	}
	
	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
		{
			if (_rsslBuffer.length() == 0)
				return DataImpl.EMPTY_STRING;
			else
				return _rsslBuffer.toString();
		}
	}

	@Override
	int decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		if (_rsslBuffer.decode(dIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;

		return com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
	}
}