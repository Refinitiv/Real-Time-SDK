/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.transportperf;

/** Transport performance test role. */
public class TransportTestRole
{
	public static final int UNINIT		= 0x00;
	public static final int WRITER		= 0x01;
	public static final int READER		= 0x02;
	public static final int REFLECTOR	= 0x04;

}
