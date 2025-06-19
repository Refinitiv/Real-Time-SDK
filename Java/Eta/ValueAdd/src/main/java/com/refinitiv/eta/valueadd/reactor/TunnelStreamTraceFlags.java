/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/* Flags for use with TunnelStream#enableTrace */

class TunnelStreamTraceFlags
{
	/* None. */
	static final int NONE		= 0x0;

	/* Trace actions taken by the tunnel stream. */
	static final int ACTIONS		= 0x1;

	/* Print an XML-style trace of messages queued, sent, and received by the queue. */
	static final int MSGS		= 0x2;

	/* Print full XML traces of messages (requires TunnelStreamTraceFlags.MSGS) */
	static final int MSG_XML		= 0x4;
}

