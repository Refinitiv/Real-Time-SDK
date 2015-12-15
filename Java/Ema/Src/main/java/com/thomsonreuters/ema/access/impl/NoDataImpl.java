///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.DataType.DataTypes;

class NoDataImpl extends DataImpl implements ComplexType
{
	@Override
	public int dataType()
	{
		return DataTypes.NO_DATA;
	}

	@Override
	public int code()
	{
		return DataCode.NO_CODE;
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object obj)
	{
		_rsslMajVer = majVer;
		_rsslMinVer = minVer;
		
		_rsslBuffer = rsslBuffer;
	}
	
	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("NoData\n");
		Utilities.addIndent(_toString, indent).append("NoDataEnd\n");
		
		return _toString.toString();
	}
}