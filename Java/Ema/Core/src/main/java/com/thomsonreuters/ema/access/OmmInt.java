///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmInt represents 64-bit value in Omm.
 * 
 * OmmInt is a read only class.
 * 
 * @see Data
 */
public interface OmmInt extends Data
{
	/**
	 * @return long
	 */
	public long intValue();
}