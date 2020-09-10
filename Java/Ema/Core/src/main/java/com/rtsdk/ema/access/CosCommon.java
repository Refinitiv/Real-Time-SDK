/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */
package com.rtsdk.ema.access;

/*
 * CosCommon describes common options related to the exchange of messages.
 */
public interface CosCommon {
	/*
	 * Clears the CosCommon object
	 */
	public CosCommon clear();

	/*
	 * Specifies the maximum size of messages exchanged on the tunnel stream.
	 */
	public CosCommon maxMsgSize(int maxMsgSize);
	
	/*
	 * Returns the maximum message size assigned by the provider accepting the tunnel stream request.
	 */
	public int maxMsgSize();

}
