/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Text;

namespace LSEG.Eta.Example.Common
{
    public class PingHandler
    {
        public int PingTimeoutRemote { get; set; } = 0;
        public int PingTimeoutLocal { get; set; } = 0;
        public long NextRemotePingTime { get; set; } = 0;
        public long NextLocalPingTime { get; set; } = 0;

        /// <summary>
        /// Indicates that we received a message from the remote connection.
        /// </summary>
        public bool ReceivedRemoteMsg { get; set; } = false;

        /// <summary>
        /// Indicates that a message was sent to remote connection.
        /// </summary>
        public bool SentLocalMsg { get; set; } = false;

        /// <summary>
        /// Initializes the ping times for a channel.
        /// </summary>
        /// <param name="timeout">timeout for local and remote pings (in seconds)</param>
        public void InitPingHandler(int timeout)
        {
            // set ping timeout for local and remote pings
            PingTimeoutLocal = timeout / 3;
            PingTimeoutRemote = timeout;

            // set time to send next ping to remote connection
            NextLocalPingTime = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond + PingTimeoutLocal * 1000;
            // set time should receive next ping from remote connection
            NextRemotePingTime = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond + PingTimeoutRemote * 1000;
        }

        /// <summary>
        /// Handles the ping processing for a channel.
        /// <ol>
        /// <li>Sends a ping to the remote(connection) if the next local ping time 
        /// has expired and a local message was not sent to the remote(connection).</li>
        /// <li>Checks if a ping has been received from the remote(connection) within the next receive ping time.</li>
        /// </ol>
        /// </summary>
        /// <param name="chnl">The channel to handle pings for.</param>
        /// <param name="error">Error information when send ping fails.</param>
        /// <returns><see cref="TransportReturnCode.SUCCESS"/> for successful sending of the ping. 
        /// Less than <see cref="TransportReturnCode.FAILURE"/> for failure.</returns>
        public TransportReturnCode HandlePings(IChannel chnl, out Error? error)
        {
            long currentTime = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;

            // handle local pings
            if (currentTime >= NextLocalPingTime)
            {
                // check if local message was sent to the remote (connection) since last time
                if (SentLocalMsg)
                {
                    SentLocalMsg = false;
                }
                else
                {
                    // send ping to remote (connection)
                    TransportReturnCode ret = chnl.Ping(out error);
                    if (ret < TransportReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                // set time to send next local ping
                NextLocalPingTime = currentTime + PingTimeoutLocal * 1000;
            }

            // handle remote pings
            if (currentTime >= NextRemotePingTime)
            {
                 // check if received message from remote (connection) since last time
                if (ReceivedRemoteMsg)
                {
                    // reset flag for remote message received
                    ReceivedRemoteMsg = false;

                    // set time should receive next message/ping from remote (connection)
                    NextRemotePingTime = currentTime + PingTimeoutRemote * 1000;
                }
                else
                {
                    // lost contact with remote (connection)
                    error = new Error()
                    {
                        Text = "Lost contact with connection...",
                        ErrorId = TransportReturnCode.FAILURE
                    };
                    return TransportReturnCode.FAILURE;
                }
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Re-initializes ping handler for possible reuse.
        /// </summary>
        public void Clear()
        {
            PingTimeoutRemote = 0;
            PingTimeoutLocal = 0;
            NextRemotePingTime = 0;
            NextLocalPingTime = 0;
            ReceivedRemoteMsg = false;
            SentLocalMsg = false;
        }
    }
}
