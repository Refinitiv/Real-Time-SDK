/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Event provided to ReactorAuthTokenEventCallback methods to retrieve authentication
    /// token information
    /// <seealso cref="ReactorAuthTokenInfo"/>
    /// </summary>
    sealed public class ReactorAuthTokenEvent : ReactorEvent
    {
        internal ReactorAuthTokenEvent()
        {
        }

        /// <summary>
        /// Gets the Authorization Token Info associated with this event.
        /// </summary>
        public ReactorAuthTokenInfo? ReactorAuthTokenInfo { get; internal set; }

        /// <summary>
        /// Returns this object back to the pool.
        /// </summary>
        public override void ReturnToPool()
        {
            ReactorAuthTokenInfo = null;

            base.ReturnToPool();
        }

    }
}
