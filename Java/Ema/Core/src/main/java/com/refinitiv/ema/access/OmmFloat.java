/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

/**
 * OmmFloat represents float value in Omm.
 * 
 * OmmFloat is a read only class.
 * 
 * @see Data
 */
public interface OmmFloat extends Data
{
	/**
	 * Returns Float.
	 * 
	 * @return float
	 */
	public float floatValue();
}