///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmRmtes represents Rmtes string value in Omm.
 * <p>OmmRmtes is a read only class.</p>
 */
public interface OmmRmtes extends Data
{
	/**
	 * Returns Rmtes.
	 * 
	 * @return RmtesBuffer containing Rmtes data
	 */
	public RmtesBuffer rmtes();
}