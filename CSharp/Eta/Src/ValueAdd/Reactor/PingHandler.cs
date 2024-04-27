/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Data.SqlTypes;
using System.Diagnostics;
using System.Text;

using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Ping handler for the Reactor
    /// </summary>
    internal class PingHandler
    {
        private volatile int m_PingTimeoutRemote = 0;
        private long m_PingTimeoutLocal = 0;
        private long m_NextRemotePingTime = 0;
        private long m_NextLocalPingTime = 0;
        private long m_PingsReceived = 0;
        private long m_PingsSent = 0;
        private volatile bool m_ReceivedRemoteMsg = false;
        private volatile bool m_SentLocalMsg = false;
        private static bool TRACK_PINGS = false;

        private StringBuilder m_XmlString = new StringBuilder(512);

        /// <summary>
        /// Re-initializes ping handler for possible reuse.
        /// </summary>
        internal void Clear()
        {
            m_PingTimeoutRemote = 0;
            m_PingTimeoutLocal = 0;
            m_NextRemotePingTime = 0;
            m_NextLocalPingTime = 0;
            m_PingsReceived = 0;
            m_PingsSent = 0;
            m_XmlString.Length = 0;
            m_ReceivedRemoteMsg = false;
            m_SentLocalMsg = false;
        }

        /// <summary>
        /// Indicates that we received a message from the remote connection
        /// </summary>
        internal void ReceivedMsg()
        {
            m_ReceivedRemoteMsg = true;
        }

        /// <summary>
        /// Indicates that we sent a message to remote connection
        /// </summary>
        internal void SentMsg()
        {
            m_SentLocalMsg = true;
        }

        /// <summary>
        /// Indicates that we received a message from the remote connection
        /// </summary>
        internal void ReceivedPing(ReactorChannel reactorChannel)
        {
            ReactorOptions reactorOptions = reactorChannel.Reactor!.m_ReactorOptions;

            if (reactorOptions.XmlTracePing
                && (reactorOptions.XmlTracing
                    || reactorOptions.XmlTraceToFile))
            {
                m_XmlString.Length = 0;
                m_XmlString
                    .Append("\n<!-- Incoming Ping message -->\n<!-- ")
                    .Append(reactorChannel.Channel)
                    .Append("-->\n<!-- ").Append(DateTime.Now).Append(" -->\n");

                if (reactorOptions.XmlTracing)
                    Console.WriteLine(m_XmlString);

                if (reactorOptions.XmlTraceToFile)
                    reactorChannel.Reactor!.m_FileDumper!.Dump(m_XmlString);
            }

            m_ReceivedRemoteMsg = true;

            if (TRACK_PINGS)
            {
                m_PingsReceived++;
            }
        }

        /// <summary>
        /// Indicates that we sent a message to remote connection
        /// </summary>
        internal void SentPing()
        {
            if(TRACK_PINGS)
            {
                m_PingsSent++;
            }
        }

        internal void InitPingHandler(int timeOut)
        {
            /* set ping timeout for local and remote pings */
            m_PingTimeoutLocal = timeOut / 3;
            m_PingTimeoutRemote = timeOut;

            /* set time to send next ping to remote connection */
            m_NextLocalPingTime = ReactorUtil.GetCurrentTimeMilliSecond() + m_PingTimeoutLocal * 1000;

            /* set time should receive next ping from remote connection */
            m_NextRemotePingTime = ReactorUtil.GetCurrentTimeMilliSecond() + m_PingTimeoutRemote * 1000;
        }

        /// <summary>
        /// Handles the ping processing for a channel.
        /// </summary>
        /// <remarks>
        /// Sends a ping to the remote (connection) if the next local ping time
        /// has expired and a local message was not sent to the remote (connection).
        /// 
        /// Checks if a ping has been received from the remote (connection)
        /// within the next receive ping time.
        /// </remarks>
        /// <param name="reactorChannel">The channel to handle ping</param>
        /// <param name="error">The error in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public TransportReturnCode HandlePings(ReactorChannel reactorChannel, out Error? error)
        {
            error = null;
            long currentTime = ReactorUtil.GetCurrentTimeMilliSecond();
            IChannel? channel = reactorChannel.Channel;

            /* handle local pings */
            if (currentTime >= m_NextLocalPingTime)
            {
                /*
                 * check if local message was sent to the remote (connection) since
                 * last time
                 */
                if (m_SentLocalMsg)
                {
                    m_SentLocalMsg = false;
                }
                else
                {
                    /* send ping to remote (connection) */

                    ReactorOptions reactorOptions = reactorChannel.Reactor!.m_ReactorOptions;

                    if (reactorOptions.XmlTracePing
                        && (reactorOptions.XmlTracing
                            || reactorOptions.XmlTraceToFile))
                    {
                        m_XmlString.Length = 0;

                        m_XmlString
                            .Append("\n<!-- Outgoing Ping message -->\n<!-- ")
                            .Append(reactorChannel.Channel)
                            .Append("-->\n<!-- ").Append(DateTime.Now).Append(" -->\n");

                        if (reactorOptions.XmlTracing)
                            Console.WriteLine(m_XmlString);

                        if (reactorOptions.XmlTraceToFile)
                            reactorChannel.Reactor!.m_FileDumper!.Dump(m_XmlString);
                    }

                    TransportReturnCode ret = channel!.Ping(out error);
                    if (ret < TransportReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    else if (ret == TransportReturnCode.SUCCESS)
                    {
                        SentPing();
                    }
                }

                /* set time to send next local ping */
                m_NextLocalPingTime = currentTime + m_PingTimeoutLocal * 1000;
            }

            /* handle remote pings */
            if (currentTime >= m_NextRemotePingTime)
            {
                /*
                 * check if received message from remote (connection) since last
                 * time
                 */
                if (m_ReceivedRemoteMsg)
                {
                    /* reset flag for remote message received */
                    m_ReceivedRemoteMsg = false;

                    /*
                     * set time should receive next message/ping from remote
                     * (connection)
                     */
                    m_NextRemotePingTime = currentTime + m_PingTimeoutRemote * 1000;
                }
                else
                {
                    /* lost contact with remote (connection) */
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "Lost contact with connection..."
                    };

                    return TransportReturnCode.FAILURE;
                }
            }

            return TransportReturnCode.SUCCESS;
        }

        internal long GetPingReceived()
        {
            return m_PingsReceived;
        }

        internal long GetPingSent()
        {
            return m_PingsSent;
        }

        internal static void TrackPings(bool trackPings)
        {
            TRACK_PINGS = trackPings;
        }
    }
}
