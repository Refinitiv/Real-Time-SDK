/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    /// This is used to notify read or write events
    /// </summary>
    public class NotifierEvent
    {
        /// <summary>
        /// Indicates whether the event's Socket can be read or write
        /// </summary>
        public NotifierEventFlag NotifiedFlags { get; set; }

        /// <summary>
        /// Indicates whether the event's Socket can be read from
        /// </summary>
        /// <returns><c>true</c> if this event is readable</returns>
        public bool IsReadable()
        {
            return (NotifiedFlags & NotifierEventFlag.READ) != 0;
        }

        /// <summary>
        /// Indicates whether the event's Socket can be written to
        /// </summary>
        /// <returns><c>true</c> if this event is writeable</returns>
        public bool IsWriteable()
        {
            return (NotifiedFlags & NotifierEventFlag.WRITE) != 0;
        }

        /// <summary>
        /// Indicates whether the event's Socket may be invalid.
        /// </summary>
        /// <returns><c>true</c> if this event is bad socket</returns>
        public bool IsBadSocket()
        {
            return (NotifiedFlags & NotifierEventFlag.BAD_SOCKET) != 0;
        }

        /// <summary>
        /// Clears the event's notification flags. May be useful if you are done reading from/writing to a notified event.
        /// </summary>
        public void ClearNotifiedFlags()
        {
            NotifiedFlags = 0;
        }

        internal NotifierEventFlag _RegisteredFlags { get; set; }

        internal Object? UserSpec { get; set; }

        internal Socket? EventSocket { get; set; }

        internal Notifier? Notifier { get; set; }
    }
}
