///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.nio.ByteBuffer;

import com.rtsdk.ema.access.EmaUtility;
import com.rtsdk.eta.codec.CodecReturnCodes;

class OmmBufferImpl extends DataImpl implements OmmBuffer
{
	OmmBufferImpl()
	{
		_rsslBuffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
	}
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.BUFFER;
	}

	@Override
	public ByteBuffer buffer()
	{
		return asHex();
	}

	@Override
	public String toString()
	{
		if (code() == DataCode.BLANK)
			return BLANK_STRING;
		
		return EmaUtility.asHexString(asHex());
	}
	
	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, com.rtsdk.eta.codec.DecodeIterator dIter)
	{
		if (_rsslBuffer.decode(dIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;
	}
}