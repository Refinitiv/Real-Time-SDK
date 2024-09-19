///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.EmaUtility;
import com.refinitiv.eta.codec.CodecReturnCodes;

class OmmBufferImpl extends DataImpl implements OmmBuffer
{
	OmmBufferImpl()
	{
		_rsslBuffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
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
	int decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		if (_rsslBuffer.decode(dIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;

		return com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
	}
}