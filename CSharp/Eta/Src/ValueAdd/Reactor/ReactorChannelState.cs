/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// The ReactorChannel's state.
    /// </summary>
    public enum ReactorChannelState
    {
        /// <summary>
        /// The ReactorChannel is in unknown state. This is the initial state before the channel is used.
        /// </summary>
        UNKNOWN = 0,

        /// <summary>
        /// The ReactorChannel is initializing its connection.
        /// </summary>
        INITIALIZING = 1,

        /// <summary>
        /// The ReactorChannel connection is up.
        /// </summary>
        UP = 2,

        /// <summary>
        /// The ReactorChannel connection is ready. It has received all messages configured for its role.
        /// </summary>
        READY = 3,

        /// <summary>
        /// The ReactorChannel connection is down and there is no connection recovery.
        /// </summary>
        DOWN = 4,

        /// <summary>
        /// The ReactorChannel connection is down and connection recovery will be started.
        /// </summary>
        DOWN_RECONNECTING = 5,

        /// <summary>
        /// The ReactorChannel connection is closed and the channel is no longer usable.
        /// </summary>
        CLOSED = 6,

        /// <summary>
        /// The ReactorChannel connection is waiting for authentication and 
        /// service discovery (if host and port are not specified).
        /// </summary>
        RDP_RT = 7,

        /// <summary>
        /// The ReactorChannel connection is done for session management.
        /// </summary>
        RDP_RT_DONE = 8,

        /// <summary>
        /// The ReactorChannel failed to establish a connection for session management.
        /// </summary>
        RDP_RT_FAILED = 9
    }
}
