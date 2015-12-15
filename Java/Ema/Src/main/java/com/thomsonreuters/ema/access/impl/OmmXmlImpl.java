///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.OmmXml;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;


class OmmXmlImpl extends DataImpl implements OmmXml
{
	OmmXmlImpl()
	{
		_rsslBuffer = CodecFactory.createBuffer();
	}

	@Override
	public int dataType()
	{
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public String string()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ByteBuffer buffer()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmXml clear()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmXml string(String value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmXml buffer(ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("Xml");

		++indent;
		Utilities.addIndent(_toString.append("\n"), indent);
		if (DataCode.BLANK == code())
			_toString.append(BLANK_STRING);
		else
			_toString.append(_rsslBuffer.toString());
		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("XmlEnd\n");

		return _toString.toString();
	}
	
	@Override
	void decode(Buffer rsslBuffer, int majVer, int minVer, DataDictionary rsslDictionary, Object obj)
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
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		if (_rsslBuffer.decode(dIter) == CodecReturnCodes.SUCCESS)
			_dataCode = DataCode.NO_CODE;
		else
			_dataCode = DataCode.BLANK;
	}
}