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
    /// Represents authentication token information.
    /// <seealso cref="ReactorAuthTokenEvent"/>
    /// </summary>
    sealed public class ReactorAuthTokenInfo
    {
        /// <summary>
        /// Gets an access token information provided by the authentication service.
        /// </summary>
        public string? AccessToken { get; internal set; }

        /// <summary>
        /// Gets an access token validity time in seconds.
        /// </summary>
        public int ExpiresIn { get; internal set; }
        
        /// <summary>
        /// Gets a token type for specifying in the Authentication header.
        /// </summary>
        public string? TokenType { get; internal set; }

        /// <summary>
        /// Gets a list of all the scopes this token can be used with.
        /// </summary>
        public string? Scope { get; internal set; }

        internal ReactorAuthTokenInfo()
        {
            Clear();
        }

        internal void Clear()
        {
            AccessToken = string.Empty;
            ExpiresIn = -1;
            Scope = string.Empty;
            TokenType = string.Empty;
        }

        internal void SetAccessTokenInfo(AccessTokenInformation accessTokenInfo)
        {
            AccessToken = accessTokenInfo.access_token;
            ExpiresIn = accessTokenInfo.expires_in;
            TokenType = accessTokenInfo.token_type;
        }

        /// <summary>
        /// The string representation of this object
        /// </summary>
        /// <returns>The string value.</returns>
        public override string ToString()
        {
            return "ReactorAuthTokenInfo" + "\n" +
                   "\taccessToken: " + AccessToken + "\n" +
                   "\texpiresIn: " + ExpiresIn + "\n" +
                   "\tscope: " + Scope + "\n" +
                   "\ttokenType: " + TokenType;
        }
    }
}
