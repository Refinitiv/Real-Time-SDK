/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import com.refinitiv.eta.transport.TransportBuffer;

/** Callback used for processing a response message. */
public interface ResponseCallback
{
	
	/**
	 *  Process the response message.
	 *
	 * @param buffer the buffer
	 */
    public void processResponse(TransportBuffer buffer);
}
