package com.thomsonreuters.upa.valueadd.reactor;

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

