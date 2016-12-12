package com.thomsonreuters.upa.shared;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.Error;

/** The ping handler for the example applications and performance tools. */
public class PingHandler
{
    private int pingTimeoutRemote = 0;
    private int pingTimeoutLocal = 0;
    private long nextRemotePingTime = 0;
    private long nextLocalPingTime = 0;
    private boolean receivedRemoteMsg = false;
    private boolean sentLocalMsg = false;

    /**
     * Ping timeout for local connection.
     * 
     * @return timeout value in seconds
     */
    public int localTime()
    {
        return pingTimeoutLocal;
    }

    /**
     * Ping timeout for remote connection.
     * 
     * @return timeout value in seconds
     */
    public int remoteTimeout()
    {
        return pingTimeoutRemote;
    }

    /**
     * Indicate that we received a message from the remote connection
     */
    public void receivedMsg()
    {
        receivedRemoteMsg = true;
    }

    /**
     * Indicate that we sent a message to remote connection
     */
    public void sentMsg()
    {
        sentLocalMsg = true;
    }

    /**
     * Initializes the ping times for a channel.
     * 
     * @param timeout timeout for local and remote pings (in seconds)
     */
    public void initPingHandler(int timeout)
    {
        /* set ping timeout for local and remote pings */
        pingTimeoutLocal = timeout / 3;
        pingTimeoutRemote = timeout;

        /* set time to send next ping to remote connection */
        nextLocalPingTime = System.currentTimeMillis() + pingTimeoutLocal * 1000;

        /* set time should receive next ping from remote connection */
        nextRemotePingTime = System.currentTimeMillis() + pingTimeoutRemote * 1000;
    }

    /**
     * Handles the ping processing for a channel.
     * <p>
     * <ol>
     * <li>Sends a ping to the remote (connection) if the next local ping time
     * has expired and a local message was not sent to the remote (connection).</li>
     * <li>Checks if a ping has been received from the remote (connection)
     * within the next receive ping time.</li>
     * </ol>
     * 
     * @param chnl The channel to handle pings for.
     * @param error Error information when send ping fails.
     * 
     * @return {@link TransportReturnCodes#SUCCESS} for successful sending of
     *         ping. Less than {@link TransportReturnCodes#SUCCESS} for failure.
     */
    public int handlePings(Channel chnl, Error error)
    {
        long currentTime = System.currentTimeMillis();

        /* handle local pings */
        if (currentTime >= nextLocalPingTime)
        {
            /*
             * check if local message was sent to the remote (connection) since
             * last time
             */
            if (sentLocalMsg)
            {
                sentLocalMsg = false;
            }
            else
            {
                /* send ping to remote (connection) */
                int ret = chnl.ping(error);
                if (ret < TransportReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }

            /* set time to send next local ping */
            nextLocalPingTime = currentTime + pingTimeoutLocal * 1000;
        }

        /* handle remote pings */
        if (currentTime >= nextRemotePingTime)
        {
            /*
             * check if received message from remote (connection) since last
             * time
             */
            if (receivedRemoteMsg)
            {
                /* reset flag for remote message received */
                receivedRemoteMsg = false;

                /*
                 * set time should receive next message/ping from remote
                 * (connection)
                 */
                nextRemotePingTime = currentTime + pingTimeoutRemote * 1000;
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

    /**
     * Re-initializes ping handler for possible reuse.
     */
    public void clear()
    {
        pingTimeoutRemote = 0;
        pingTimeoutLocal = 0;
        nextRemotePingTime = 0;
        nextLocalPingTime = 0;
        receivedRemoteMsg = false;
        sentLocalMsg = false;
    }

}
