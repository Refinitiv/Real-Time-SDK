/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

/** Item flags. */
public class ItemFlags
{
	/** Provider should send updates */
	public static final int IS_STREAMING_REQ	= 0x04;
	/** Item was requested(not published) */
	public static final int IS_SOLICITED		= 0x10;
	/** Consumer should send posts */
	public static final int IS_POST				= 0x20;
	/** Consumer should send generic msgs */
	public static final int IS_GEN_MSG			= 0x40;
	/** Consumer should request private stream */
	public static final int IS_PRIVATE			= 0x80;
}
