/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.consperf;

/** Item request state. */
public class ItemRequestState
{
	/** Item request has not been set. */
	public static final int NOT_REQUESTED = 0;
	/** Item is waiting for its solicited refresh. */
	public static final int WAITING_FOR_REFRESH = 1;
	/** Item has received its solicited refresh. */
	public static final int HAS_REFRESH = 2;
}
