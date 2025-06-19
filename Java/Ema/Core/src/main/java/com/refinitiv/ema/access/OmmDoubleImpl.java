/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;


class OmmDoubleImpl extends DataImpl implements OmmDouble
{
	private com.refinitiv.eta.codec.Double _rsslDouble = com.refinitiv.eta.codec.CodecFactory.createDouble();
	
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
	int decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS == _rsslDouble.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslDouble.blank();
		}
		return com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
	}
}