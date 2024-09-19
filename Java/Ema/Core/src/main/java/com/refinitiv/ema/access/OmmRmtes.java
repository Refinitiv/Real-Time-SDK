///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmRmtes represents Rmtes string value in Omm.<br>
 * 
 * OmmRmes is a read only class.
 *
 * @see Data
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