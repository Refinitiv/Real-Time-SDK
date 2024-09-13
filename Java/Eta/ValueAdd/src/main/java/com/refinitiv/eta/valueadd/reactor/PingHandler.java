/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022, 2024 Refinitiv. All rights reserved.   --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.Error;

/* Ping handler for the Reactor */
class PingHandler
{
    private volatile int _pingTimeoutRemote = 0;
    private volatile int _pingTimeoutLocal = 0;
    private volatile long _nextRemotePingTime = 0;
    private volatile long _nextLocalPingTime = 0;
    private volatile long _pingsReceived = 0;
    private volatile long _pingsSent = 0;
    private volatile boolean _receivedRemoteMsg = false;
    private volatile boolean _sentLocalMsg = false;
    private static boolean _trackPings = true;
    private StringBuilder xmlString = new StringBuilder(1500);
    ReactorOptions _reactorOptions;

    /*
     * Indicate that we received a message from the remote connection
     */
    void receivedMsg()
    {
        _receivedRemoteMsg = true;
    }

    /*
     * Indicate that we sent a message to remote connection
     */
    void sentMsg()
    {
        _sentLocalMsg = true;
    }

    /*
     * Indicate that we received a message from the remote connection
     */
    void receivedPing()
    {
        if (_trackPings)
            _pingsReceived++;
    }

    /*
     * Indicate that we sent a message to remote connection
     */
    void sentPing()
    {
        if (_trackPings)
            _pingsSent++;
    }

    /*
     * Initializes the ping times for a channel.
     */
    void initPingHandler(int timeout)
    {
        /* set ping timeout for local and remote pings */
        _pingTimeoutLocal = timeout / 3;
        _pingTimeoutRemote = timeout;

        /* set time to send next ping to remote connection */
        _nextLocalPingTime = System.currentTimeMillis() + _pingTimeoutLocal * 1000;

        /* set time should receive next ping from remote connection */
        _nextRemotePingTime = System.currentTimeMillis() + _pingTimeoutRemote * 1000;
    }

    /*
     * Handles the ping processing for a channel.
     *
     * Sends a ping to the remote (connection) if the next local ping time
     * has expired and a local message was not sent to the remote (connection).
     *
     * Checks if a ping has been received from the remote (connection)
     * within the next receive ping time.
     */
    public int handlePings(ReactorChannel reactorChannel, Error error)
    {
        long currentTime = System.currentTimeMillis();
        Channel chnl = reactorChannel.channel();

        /* handle local pings */
        if (currentTime >= _nextLocalPingTime)
        {
            boolean isJSONProtocol = chnl.protocolType() == Codec.JSON_PROTOCOL_TYPE ? true : false;
            /*
             * check if local message was sent to the remote (connection) since
             * last time
             */
            if (_sentLocalMsg && !isJSONProtocol)
            {
                _sentLocalMsg = false;
            }
            else
            {
                boolean sendPing = true;

                if (isJSONProtocol && reactorChannel.sendPingMessage() == false)
                {
                    sendPing = false;
                }

                if(sendPing)
                {
                    /* send ping to remote (connection) */
                    _reactorOptions = reactorChannel.reactor().reactorOptions();

                    if (_reactorOptions.xmlTracePing() && (_reactorOptions.xmlTracing() || _reactorOptions.xmlTraceToFile())) 
                    {
                        xmlString.setLength(0);
                        xmlString.append("\n<!-- Outgoing Ping message -->\n").append("<!-- ")
                                 .append(reactorChannel.selectableChannel().toString()).append(" -->\n").append("<!-- ")
                                 .append(new java.util.Date()).append(" -->\n");
                        if (_reactorOptions.xmlTracing())
                        {
                            System.out.println(xmlString);
                        }
                        if (_reactorOptions.xmlTraceToFile())
                        {
                            reactorChannel.reactor()._fileDumper.dump(xmlString.toString());
                        }
                    }
                    int ret = chnl.ping(error);
                    if (ret < TransportReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                    else if (ret == TransportReturnCodes.SUCCESS)
                    {
                        sentPing();
                    }
                }
            }

            /* set time to send next local ping */
            _nextLocalPingTime = currentTime + _pingTimeoutLocal * 1000;
        }

        /* handle remote pings */
        if (currentTime >= _nextRemotePingTime)
        {
            /*
             * check if received message from remote (connection) since last
             * time
             */
            if (_receivedRemoteMsg)
            {
                /* reset flag for remote message received */
                _receivedRemoteMsg = false;

                /*
                 * set time should receive next message/ping from remote
                 * (connection)
                 */
                _nextRemotePingTime = currentTime + _pingTimeoutRemote * 1000;
            }
            else
            {
                /* lost contact with remote (connection) */
                error.text("Lost contact with connection...");
                return TransportReturnCodes.FAILURE;
            }
        }

        return TransportReturnCodes.SUCCESS;
    }

    /*
     * access pings received
     */
    long getPingsReceived()
    {
        return _pingsReceived;
    }

    /*
     * access pings sent
     */
    long getPingsSent()
    {
        return _pingsSent;
    }

    void trackPings(boolean trackPings)
    {
        _trackPings = trackPings;
    }

    /*
     * Re-initializes ping handler for possible reuse.
     */
    void clear()
    {
        _pingTimeoutRemote = 0;
        _pingTimeoutLocal = 0;
        _nextRemotePingTime = 0;
        _nextLocalPingTime = 0;
        _pingsReceived = 0;
        _pingsSent = 0;
        _receivedRemoteMsg = false;
        _sentLocalMsg = false;
    }

    /*
     * Resets aggregated metrics only
     */
    void resetAggregatedStats()
    {
        _pingsReceived = 0;
        _pingsSent = 0;
    }
}
