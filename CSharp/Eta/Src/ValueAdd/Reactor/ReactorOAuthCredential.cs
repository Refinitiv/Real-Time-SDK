/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
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
        /// The default value for the audience claim string for JWT OAuth2 interactions.
        /// </summary>
        public static readonly string DEFAULT_JWT_AUDIENCE = "https://login.ciam.refinitiv.com/as/token.oauth2";

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
            ClientJwk.Clear();
            Audience.Data(DEFAULT_JWT_AUDIENCE);
            TokenScope.Data("trapi.streaming.pricing.read");
            ReactorOAuthCredentialEventCallback = null;
            UserSpecObj = null;
        }

        /// <summary>
        /// Gets or sets the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
        /// </summary>
        public Buffer ClientId { get; private set; } = new Buffer();

        /// <summary>
        /// Gets or sets the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
        /// </summary>
        public Buffer ClientSecret { get; private set; } = new Buffer();

        /// <summary>
        /// Gets or sets the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the RDP token service. Mandatory for V2 logins with client JWT logins 
        /// </summary>
        public Buffer ClientJwk { get; private set; } = new Buffer();

        /// <summary>
        /// Gets or sets the audience claim for the JWT. Optional and only used for V2 Client Credentials with JWT.
        /// Optionally specifies the audience for the JWT usage.
        /// </summary>
        public Buffer Audience { get; private set; } = new Buffer();

        /// <summary>
        /// Gets or sets the token scope to limit the scope of generated token. Optional.
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

            if(ClientJwk.Length != 0)
            {
                ByteBuffer byteBuffer = new(ClientJwk.Length);
                ClientJwk.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredential.ClientJwk.Data(byteBuffer);
            }

            if (Audience.Length != 0)
            {
                ByteBuffer byteBuffer = new(Audience.Length);
                Audience.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredential.Audience.Data(byteBuffer);
            }

            destReactorOAuthCredential.ReactorOAuthCredentialEventCallback = ReactorOAuthCredentialEventCallback;
            destReactorOAuthCredential.UserSpecObj = UserSpecObj;

            return ReactorReturnCode.SUCCESS;
        }

        internal JsonWebKey? JsonWebKey { get; set; } /* This is used for authenticating with PING JWT */
    }
}
