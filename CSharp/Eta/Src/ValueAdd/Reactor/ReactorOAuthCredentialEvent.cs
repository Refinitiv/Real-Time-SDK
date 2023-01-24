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
    /// An event that has occurred on a token session to get sensitive information from the application.
    /// </summary>
    /// <seealso cref="ReactorEvent"/>
    sealed public class ReactorOAuthCredentialEvent : ReactorEvent
    {
        /// <summary>
        /// Gets the OAuth credential renewal information associated with this event.
        /// </summary>
        public ReactorOAuthCredentialRenewal? ReactorOAuthCredentialRenewal { get; internal set; }

        /// <summary>
        /// Gets the Reactor associated with this event.
        /// </summary>
        public Reactor? Reactor { get; internal set; }

        /// <summary>
        /// Gets the UserSpecObj specified in <see cref="ReactorOAuthCredential"/>
        /// </summary>
        public object? UserSpecObj { get; internal set; }

        /// <summary>
        /// Returns this object back to its pool.
        /// </summary>
        public override void ReturnToPool()
        {
            UserSpecObj = null;
            Reactor = null;
            ReactorOAuthCredentialRenewal = null;

            base.ReturnToPool();
        }
    }
}
