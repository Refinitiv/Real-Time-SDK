///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

class AllocatedMemory
{
	private AllocatedMemory()
	{
		throw new AssertionError();
	}
	
	public final static int UNKNOWN = 0x00;
	public final static int NAME = 0x01;
	public final static int ENC_ATTRIB = 0x02;
	public final static int ENC_MSG_BUFFER = 0x04;

}