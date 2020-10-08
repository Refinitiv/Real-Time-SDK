///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.EmaUtility;
import com.refinitiv.eta.codec.CodecReturnCodes;

class OmmAnsiPageImpl extends DataImpl implements OmmAnsiPage
{
	OmmAnsiPageImpl()
	{
		_rsslBuffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
	}
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.ANSI_PAGE;
	}

	@Override
	public ByteBuffer buffer()
	{
		return asHex();
	}

	@Override
	public String string()
	{
		if (_rsslBuffer.length() == 0)
			return DataImpl.EMPTY_STRING;
		else
			return _rsslBuffer.toString();
	}

	@Override
	public String toString()
	{
		return toString(0);
	}
	
	@Override
	public OmmAnsiPage string(String value)
	{
		_rsslBuffer.data(value);
		return this;
	}
	
	@Override
	public OmmAnsiPage buffer(ByteBuffer value)
	{
		Utilities.copy(value, _rsslBuffer);
		return this;
	}
	
	@Override
	public OmmAnsiPage clear()
	{
		ByteBuffer data = _rsslBuffer.data();
		if (data != null)
		{
			data.clear();
			_rsslBuffer.data(data);
		}
		else
			_rsslBuffer.clear();
		
		return this;
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);

		Utilities.addIndent(_toString, indent);
		_toString.append("AnsiPage\n\n").append(EmaUtility.asHexString(asHex()));
		Utilities.addIndent(_toString.append("\n"), indent).append("AnsiPageEnd\n");
		
		return _toString.toString();
	}

	@Override
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.refinitiv.eta.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		_rsslDecodeIter.clear();
		
		if ((_rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer)) != CodecReturnCodes.SUCCESS)
		{
			_dataCode = DataCode.BLANK;
			return;
		}

		if (_rsslBuffer.decode(_rsslDecodeIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;
	}

	@Override
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		if (_rsslBuffer.decode(dIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;
	}
}