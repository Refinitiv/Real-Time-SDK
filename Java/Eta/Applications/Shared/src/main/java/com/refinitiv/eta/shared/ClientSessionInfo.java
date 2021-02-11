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
