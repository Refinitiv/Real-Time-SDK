///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/*
 * CosDataIntegrity contains options related to the reliability of content exchanged over the tunnel stream.
 */
public interface CosDataIntegrity
{
	public class CosDataIntegrityType
	{
		public final static int BEST_EFFORT = 0;
		public final static int RELIABLE = 1;
	}
	
	/*
	 * Clears the CosDataIntegrity object
	 */
	public CosDataIntegrity clear();
	
	/*
	 * Specifies data integrity type
	 */
	public CosDataIntegrity type(int type);
	
	/*
	 * Returns data integrity type
	 */
	public int type();
}
