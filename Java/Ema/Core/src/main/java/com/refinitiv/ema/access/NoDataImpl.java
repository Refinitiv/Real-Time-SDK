///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


class NoDataImpl extends DataImpl implements ComplexType
{
	@Override
	public int dataType()
	{
		return DataType.DataTypes.NO_DATA;
	}

	@Override
	public int code()
	{
		return DataCode.NO_CODE;
	}

	@Override
	public String toString()
	{
		return toString(0);
	}
	
	@Override
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.refinitiv.eta.codec.DataDictionary rsslDictionary, Object obj)
	{
		_rsslMajVer = majVer;
		_rsslMinVer = minVer;
		_rsslBuffer = rsslBuffer;
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("NoData\n");
		Utilities.addIndent(_toString, indent).append("NoDataEnd\n");
		
		return _toString.toString();
	}
}