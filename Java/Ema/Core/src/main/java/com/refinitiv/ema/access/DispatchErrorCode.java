///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * Dispatch error codes from {@link Reactor#dispatchAll(Set, ReactorDispatchOptions, ReactorErrorInfo)} method
 */
public class DispatchErrorCode {
	/**
	 * General failure.
	 */
	public final static int FAILURE = -1;

	/**
	 * Reactor is shutdown.
	 */
	public final static int SHUTDOWN = -10;
}