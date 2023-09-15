/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ReactorSubmitOptions class is used while submitting a message
    /// </summary>
    sealed public class ReactorSubmitOptions
    {
        /// <summary>
        /// Service name to be associated with the message, 
        /// if specifying the service by name instead of by ID (watchlist enabled only).
        /// </summary>
        public string? ServiceName { get; set; } = null;

        /// <summary>
        /// <see cref="WriteArgs"/> used to write a message
        /// </summary>
        public WriteArgs WriteArgs { get; set; } = new WriteArgs();

        /// <summary>
        /// Gets request message options if submitting RequestMsg and enabling the watchlist.
        /// </summary>
        public ReactorRequestMsgOptions RequestMsgOptions { get; private set; } = new ReactorRequestMsgOptions();

        /// <summary>
        /// Clears to default values.
        /// </summary>
        public void Clear()
        {
            ServiceName = null;
            WriteArgs.Clear();
            RequestMsgOptions.Clear();
        }
    }
}
