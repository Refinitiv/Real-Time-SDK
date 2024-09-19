/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Channel states
    /// </summary>
    public enum ChannelState
    {
        /// <summary>
        /// Channel has been CLOSED.
        /// Channel is set in this state when any socket related operation failed 
        /// because the far end connection has been closed.
        /// </summary>
        CLOSED = -1,

        /// <summary>
        /// Indicates that a channel is inactive. This channel cannot be used. 
        /// This state typically occurs after a channel is closed by the user.
        /// </summary>
        INACTIVE = 0,

        /// <summary>
        /// Indicates that a channel requires additional initialization.
        /// </summary>
        INITIALIZING = 1,

        /// <summary>
        /// Indicates that a channel is active.
        /// </summary>
        ACTIVE = 2
    }
}
