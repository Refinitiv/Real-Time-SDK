///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;


class OmmEnumImpl extends DataImpl implements OmmEnum
{
	private com.rtsdk.eta.codec.Enum _rsslEnum = com.rtsdk.eta.codec.CodecFactory.createEnum();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.ENUM;
	}

	@Override
	public int enumValue()
	{
		return _rsslEnum.toInt();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslEnum.toString();
	}

	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, com.rtsdk.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.rtsdk.eta.codec.CodecReturnCodes.SUCCESS == _rsslEnum.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslEnum.clear();
		}
	}
}