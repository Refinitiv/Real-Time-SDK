/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.common;

import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/**
 * Methods handling message sending used by the ETA Value Add example applications.
 */
public class SendMessage {
	public static int sendMessage(ReactorChannel chnl, TransportBuffer msgBuf, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
	{
		int ret = chnl.submit(msgBuf, submitOptions, errorInfo);
		
		while (ret == ReactorReturnCodes.WRITE_CALL_AGAIN)
			ret = chnl.submit(msgBuf, submitOptions, errorInfo);
		
		if (ret < ReactorReturnCodes.SUCCESS)
		{
			chnl.releaseBuffer(msgBuf, errorInfo);
		}
		
		return ret;
	}
}
