/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import com.refinitiv.eta.transport.Channel;

/**
 * Client session information.
 */
public class ClientSessionInfo
{
    Channel clientChannel;
    PingHandler pingHandler = new PingHandler();
    long start_time;
    JsonSession jsonSession = new JsonSession();
    int socketFdValue = -1;

    public Channel clientChannel()
    {
        return clientChannel;
    }

    public void clear()
    {
        clientChannel = null;
        pingHandler.clear();
        start_time = 0;
        socketFdValue = -1;
        jsonSession.clear();
    }

    public long startTime()
    {
        return start_time;
    }

    public int socketFdValue() {
        return socketFdValue;
    }

    public JsonSession getJsonSession() {
        return jsonSession;
    }
}
