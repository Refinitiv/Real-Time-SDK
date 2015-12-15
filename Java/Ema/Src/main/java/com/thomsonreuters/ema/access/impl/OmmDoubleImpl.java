///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmDouble;
import com.thomsonreuters.upa.codec.CodecFactory;

class OmmDoubleImpl extends DataImpl implements OmmDouble
{
	private com.thomsonreuters.upa.codec.Double _rsslDouble = CodecFactory.createDouble();
	
	@Override
	public int dataType()
	{
		return DataTypes.DOUBLE;
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
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS == _rsslDouble.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslDouble.blank();
		}
	}
}