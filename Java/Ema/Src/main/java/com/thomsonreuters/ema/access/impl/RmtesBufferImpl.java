///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

import com.thomsonreuters.ema.access.OmmRmtes;
import com.thomsonreuters.ema.access.RmtesBuffer;
import com.thomsonreuters.upa.codec.CodecFactory;

public class RmtesBufferImpl implements RmtesBuffer
{
	private com.thomsonreuters.upa.codec.RmtesBuffer _rsslRmtesBuffer = CodecFactory.createRmtesBuffer(0);
	
	public RmtesBufferImpl()
	{
		
	}

	@Override
	public CharBuffer asUTF8()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void asUTF8(CharBuffer outBuffer)
	{
		// TODO Auto-generated method stub
		
	}

	@Override
	public CharBuffer asUTF16()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void asUTF16(CharBuffer outBuffer)
	{
		// TODO Auto-generated method stub
		
	}

	@Override
	public RmtesBuffer clear()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public RmtesBuffer apply(RmtesBuffer data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public RmtesBuffer apply(OmmRmtes data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public RmtesBuffer setBackingBuffer(ByteBuffer buffer)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	com.thomsonreuters.upa.codec.RmtesBuffer rsslRmtesBuffer()
	{
		return _rsslRmtesBuffer;
	}
}