///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmRmtes;
import com.thomsonreuters.ema.access.RmtesBuffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class OmmRmtesImpl extends DataImpl implements OmmRmtes
{
	private RmtesBufferImpl	_rmtesBuffer = new RmtesBufferImpl();
	
	OmmRmtesImpl()
	{
		_rsslBuffer = CodecFactory.createBuffer();
	}
	
	@Override
	public int dataType()
	{
		return DataTypes.RMTES;
	}

	@Override
	public String toString()
	{
		//TODO
		if (code() == DataCode.BLANK)
			return BLANK_STRING;
		else
			return _rsslBuffer.toString();
	}
	
	@Override
	public RmtesBuffer rmtes()
	{
		return _rmtesBuffer;
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		//TODO
		if (_rsslBuffer.decode(dIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;
	}
}