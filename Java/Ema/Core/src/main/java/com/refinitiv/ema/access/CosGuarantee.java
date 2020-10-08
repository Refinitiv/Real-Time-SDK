///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/*
 * CosGuarantee contains options related to the guarantee of content submitted over the tunnel stream.
 */
public interface CosGuarantee
{
	public class CosGuaranteeType
	{
		public static final int NONE = 0;
		public static final int PERSISTENT_QUEUE = 1;
	}

	/*
	 * Clears the CosGuarantee object
	 */
	public CosGuarantee clear();

	/*
	 * Specifies guarantee type
	 */
	public CosGuarantee type(int type);

	/*
	 * Specifies if messages are persisted locally
	 */

	public CosGuarantee persistLocally(boolean persistLocally);

	/*
	 * Specifies where files containing persistent messages are stored
	 */
	public CosGuarantee persistenceFilePath(String filePath);

	/*
	 * Return guarantee type
	 */
	public int type();

	/*
	 * Return if message should persist locally
	 */
	public boolean persistedLocally();

	/*
	 * Return file path where files containing persistent message may be stored
	 */
	public String persistenceFilePath();

}
