/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports.Interfaces;
using System;

namespace Refinitiv.Eta.Transports
{
    /// <summary>
    /// Flags for the InProgress state.
    /// </summary>
    /// <seealso cref="InProgInfo"/>
    [Flags]
    public enum InProgFlags
    {
        /// <summary>
        /// Indicates that the channel initialization is still in progress and subsequent calls to
        /// <see cref="IChannel.Init(InProgInfo, out Error)"/> are required to complete it. No channel change
        /// has occurred as a result to this call.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// Indicates that a channel change has occurred as a result of this call.
        /// The previous channel has been stored in <see cref="InProgInfo.OldSocket"/> so it can be
        /// unregistered with the I/O notification mechanism. The new channel has been stored in 
        /// <see cref="InProgInfo.NewSocket"/> so it can be registered with the I/O notification
        /// mechanism. The channel initialization is still in progress and subsequent calls 
        /// <see cref="IChannel.Init(InProgInfo, out Error)"/> are required to complete it.
        /// </summary>
        SCKT_CHNL_CHANGE = 0x01
    }
}
