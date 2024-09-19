/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// ETA Channel states.
    /// </summary>
    public enum ChannelState
    {
        /// <summary>
        /// Channel has been <see cref="ChannelState.CLOSED"/>
        /// Channel is set in this state when any socket related operation failed
        /// because the far end connection has been closed.
        /// </summary>
        CLOSED = -1,

        /// <summary>
        /// Indicates that a <see cref="IChannel"/> is inactive. This channel cannot be used.
        /// This state typically occurs after a channel is closed by the user.
        /// </summary>
        INACTIVE = 0,

         /// <summary>
         /// Indicates that a <see cref="IChannel"/> requires additional initialization.
         /// This initialization is typically additional connection handshake messages
         /// that need to be exchanged. When using blocking I/O, a <see cref="IChannel"/> is
         /// typically active when it is returned and no additional initialization is
         /// required by the user.
         /// </summary>
        INITIALIZING = 1,

         /// <summary>
         /// Indicates that a <see cref="IChannel"/> is active. This channel can perform any
         /// connection related actions, such as reading or writing.
         /// </summary>
        ACTIVE = 2
    }
}
