/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Information about the stream associated with a message.
    /// </summary>
    public sealed class WatchlistStreamInfo
    {
        /// <summary>
        /// Gets or sets name of service associated with the stream, if any.
        /// </summary>
        public String? ServiceName { get; set; }

        /// <summary>
        /// Gets or sets user-specified object given when the stream was opened.
        /// </summary>
        public object? UserSpec { get; set; }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public void Clear()
        {
            ServiceName = null;
            UserSpec = null;
        }
    }
}
