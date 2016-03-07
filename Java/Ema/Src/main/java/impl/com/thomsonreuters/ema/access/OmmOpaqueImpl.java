///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.EmaUtility;
import com.thomsonreuters.ema.access.OmmOpaque;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class OmmOpaqueImpl extends DataImpl implements OmmOpaque
{
	OmmOpaqueImpl()
	{
		_rsslBuffer = CodecFactory.createBuffer();
	}
	
	@Override
	public int dataType()
	{
		return DataTypes.OPAQUE;
	}

	@Override
	public String toString()
	{
		return toString(0);
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
	public ByteBuffer buffer()
	{
		return asHex();
	}
	
		@Override
	public OmmOpaque buffer(ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmOpaque string(String value)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public OmmOpaque clear()
	{
		return this;
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);

		Utilities.addIndent(_toString, indent);
		_toString.append("Opaque\n\n").append(EmaUtility.asHexString(asHex()));
		Utilities.addIndent(_toString.append("\n"), indent).append("OpaqueEnd\n");
		
		return _toString.toString();
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
				com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		_rsslDecodeIter.clear();
		
		if ((_rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer)) != CodecReturnCodes.SUCCESS)
		{
			_dataCode = DataCode.BLANK;
			return;
		}

		if ((_rsslBuffer.decode(_rsslDecodeIter)) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;
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