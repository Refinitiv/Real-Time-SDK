///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.upa.codec.CodecReturnCodes;

class OmmRmtesImpl extends DataImpl implements OmmRmtes
{
	private RmtesBufferImpl	_rmtesBuffer;
	
	OmmRmtesImpl()
	{
		_rmtesBuffer = new RmtesBufferImpl();
	}
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.RMTES;
	}

	@Override
	public String toString()
	{
		if (code() == DataCode.BLANK)
			return BLANK_STRING;
		else
			return _rmtesBuffer.toString();
	}
	
	@Override
	public RmtesBuffer rmtes()
	{
		return _rmtesBuffer;
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		if (_rmtesBuffer.applyToCache())
			_rmtesBuffer.clear();
		
		if (_rmtesBuffer.rsslBuffer().decode(dIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;
	}
}