/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * Callback method used by {@link ProviderSession#read(Channel, com.refinitiv.eta.transport.Error, ReceivedMsgCallback)} to process client request.
 */
public interface ReceivedMsgCallback
{
    public void processReceivedMsg(Channel channel, TransportBuffer msgBuf);

    public void processChannelClose(Channel channel);
}
