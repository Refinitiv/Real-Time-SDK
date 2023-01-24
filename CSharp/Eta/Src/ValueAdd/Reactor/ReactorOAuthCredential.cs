/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using Microsoft.IdentityModel.Tokens;
using LSEG.Eta.Common;
using System.Diagnostics;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This class represents the OAuth credential for authorization with the token service.
    /// <seealso cref="ConsumerRole"/>
    /// <seealso cref="IReactorOAuthCredentialEventCallback"/>
    /// </summary>
    sealed public class ReactorOAuthCredential
    {
        /// <summary>
        /// Instantiates ReactorOAuthCredential.
        /// </summary>
        public ReactorOAuthCredential()
        {
            Clear();
        }

        /// <summary>
        /// Clears to defaults
        /// </summary>
        public void Clear()
        {
            ClientId.Clear();
            ClientSecret.Clear();
            TokenScope.Data("trapi.streaming.pricing.read");
            ReactorOAuthCredentialEventCallback = null;
            UserSpecObj = null;
        }

        /// <summary>
        /// Gets or sets an unique identifier defined for the application or user making a request to the token service.
        /// </summary>
        public Buffer ClientId { get; private set; } = new Buffer();

        /// <summary>
        /// Gets or sets a client secret that was used by OAuth Client to authenticate with the token service.
        /// </summary>
        public Buffer ClientSecret { get; private set; } = new Buffer();

        /// <summary>
        /// Gets or sets a list of token scope that is used to limit the scope of generated token.
        /// </summary>
        public Buffer TokenScope { get; private set; } = new Buffer();

        /// <summary>
        /// Gets or sets <see cref="IReactorOAuthCredentialEventCallback"/> to submit OAuth sensitive information.
        /// </summary>
        public IReactorOAuthCredentialEventCallback? ReactorOAuthCredentialEventCallback { get; set; }

        /// <summary>
        /// Gets or sets a user defined object that can be accessed via the <see cref="IReactorOAuthCredentialEventCallback"/>
        /// </summary>
        public object? UserSpecObj { get; set; }

        /// <summary>
        /// Performs a deep copy of <see cref="ReactorOAuthCredential"/> object.
        /// </summary>
        /// <param name="destReactorOAuthCredential">To copy OAuth credential object into. It cannot be <c>null</c>.</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating success or failure of copy operation.</returns>
        public ReactorReturnCode Copy(ReactorOAuthCredential destReactorOAuthCredential)
        {
            Debug.Assert(destReactorOAuthCredential is not null, "destReactorOAuthCredential must be non-null");

            if (destReactorOAuthCredential is null)
                return ReactorReturnCode.FAILURE;

            if(ClientId.Length != 0)
            {
                ByteBuffer byteBuffer = new(ClientId.Length);
                ClientId.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredential.ClientId.Data(byteBuffer);
            }

            if (ClientSecret.Length != 0)
            {
                ByteBuffer byteBuffer = new(ClientSecret.Length);
                ClientSecret.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredential.ClientSecret.Data(byteBuffer);
            }

            if (TokenScope.Length != 0)
            {
                ByteBuffer byteBuffer = new(TokenScope.Length);
                TokenScope.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredential.TokenScope.Data(byteBuffer);
            }

            destReactorOAuthCredential.ReactorOAuthCredentialEventCallback = ReactorOAuthCredentialEventCallback;
            destReactorOAuthCredential.UserSpecObj = UserSpecObj;

            return ReactorReturnCode.SUCCESS;
        }

        internal JsonWebKey? JsonWebKey { get; set; } /* This is used for authenticating with PING JWT */
    }
}
