/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// Information for the In Progress Connection State of <see cref="IChannel.Init(InProgInfo, out Error)"/>.
    /// If a backward compatibility reconnection occurs, the socket channel may change.
    /// This is how that information is relayed.
    /// </summary>
    /// <seealso cref="InProgFlags"/>
    sealed public class InProgInfo
    {
        /// <summary>
        /// Combination of bit values to indicate special behaviors and presence of optional <see cref="InProgInfo"/> content.
        /// </summary>
        /// <value>The flags</value>
        public InProgFlags Flags { get; internal set; }

        /// <summary>
        /// Old <c>Socket</c> of this ETA channel - used in Read Channel Change events.
        /// </summary>
        /// <value>The old <c>Socket</c></value>
        public Socket OldSocket { get; internal set; }

        /// <summary>
        /// <c>Socket</c> of new ETA channel
        /// </summary>
        /// <value>The new <c>Socket</c></value>
        public Socket  NewSocket { get; internal set; }

        /// <summary>
        /// Clears Information for the In Progress Connection State.
        /// </summary>
        public void Clear()
        {
            Flags = InProgFlags.NONE;
            OldSocket = null;
            NewSocket = null;
        }
    }
}
