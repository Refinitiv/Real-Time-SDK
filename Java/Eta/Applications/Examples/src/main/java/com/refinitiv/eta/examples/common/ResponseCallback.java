/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.common;

import com.refinitiv.eta.transport.TransportBuffer;

/** Callback used for processing a response message. */
public interface ResponseCallback
{
    public void processResponse(ChannelSession chnl, TransportBuffer buffer);
}
