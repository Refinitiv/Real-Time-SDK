///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access.unittest.requestrouting;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.Map;

public class ProviderTestOptions 
{
	public boolean acceptLoginRequest = true;
	
	public boolean sendRefreshAttrib = false;
	
	public boolean sendUpdateMessage = false;
	
	public boolean sendGenericMessage = false;
	
	public int sendLoginResponseInMiliSecond = 0;
	
	public int attribFlags = 0;
	
	public int featuresFlags = 0;
	
	public ByteBuffer itemGroupId = null; 
	
	public boolean sendItemResponse = true;
	
	public boolean closeItemRequest = false;
	
	public boolean supportOMMPosting = false;
	
	public boolean supportStandby = false;
	
	public int supportStandbyMode = 1;
	
	public int sendDirectoryPayloadInMiliSecond = 0;
	
	public Map sourceDirectoryPayload = null;
	
}
