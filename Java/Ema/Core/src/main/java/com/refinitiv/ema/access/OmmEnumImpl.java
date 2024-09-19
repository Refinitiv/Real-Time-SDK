///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


class OmmEnumImpl extends DataImpl implements OmmEnum
{
	private com.refinitiv.eta.codec.Enum _rsslEnum = com.refinitiv.eta.codec.CodecFactory.createEnum();
	
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
	int decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS == _rsslEnum.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslEnum.clear();
		}

		return com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
	}
}