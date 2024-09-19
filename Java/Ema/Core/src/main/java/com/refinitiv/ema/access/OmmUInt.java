///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.math.BigInteger;

/**
 * OmmUInt represents long value or BigInteger value in Omm.<br>
 * 
 * OmmUInt is a read only class.<br>
 * 
 * @see Data
 */
public interface OmmUInt extends Data
{
	/**
	 * Returns long.
	 * 
	 * @return value of long
	 */
	public long longValue();
	
	
	/**
	 * Returns BigInteger.
	 * 
	 * @return value of BigInteger
	 */
	public BigInteger bigIntegerValue();
}