///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmAscii represents Ascii string value in Omm.
 * <br> OmmAscii is a read only class.
 */
public interface OmmAscii extends Data
{
	/**
	 * @return ansi string.
	 */
	public String ascii();
}