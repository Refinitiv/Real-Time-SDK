///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/*
 * CosAuthentication contains options to authenticate a consumer to the corresponding provider
 */
public interface CosAuthentication
{
	/*
	 * An enumeration representing authentication type
	 */
	public class CosAuthenticationType
	{
		public final static int NOT_REQUIRED = 0;
		public final static int OMM_LOGIN = 1;
	}
	
	/*
	 * Clears the CosAuthentication object
	 */
	public CosAuthentication clear();

	/*
	 * Specifies the authentication type
	 */
	public CosAuthentication type(int type);
	
	/*
	 * Returns authentication type
	 */
	public int type();
}
