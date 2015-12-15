///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmAscii;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class OmmAsciiImpl extends DataImpl implements OmmAscii
{
	OmmAsciiImpl()
	{
		_rsslBuffer = CodecFactory.createBuffer();
	}
	
	@Override
	public int dataType()
	{
		return DataTypes.ASCII;
	}

	@Override
	public String ascii()
	{
		return _rsslBuffer.toString();
	}
	
	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslBuffer.toString();
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		if (_rsslBuffer.decode(dIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;
	}
}