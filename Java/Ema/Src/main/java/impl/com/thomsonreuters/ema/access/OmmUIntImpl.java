///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.math.BigInteger;


class OmmUIntImpl extends DataImpl implements OmmUInt
{
	private com.thomsonreuters.upa.codec.UInt _rsslUInt = com.thomsonreuters.upa.codec.CodecFactory.createUInt();
	
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
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS == _rsslUInt.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslUInt.clear();
		}
	}
}