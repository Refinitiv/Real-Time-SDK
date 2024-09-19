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
    /// OAuth credential renewal mode enumerations.
    /// </summary>
    public enum ReactorOAuthCredentialRenewalModes
    {
        /// <summary>
        /// Unspecified
        /// </summary>
        NONE = 0,

        /// <summary>
        /// Renew access token for clientSecret only
        /// </summary>
        CLIENT_SECRET = 1,

        /// <summary>
        /// Renew access token for clientJwk only
        /// </summary>
        CLIENT_JWK = 2,
    }
}