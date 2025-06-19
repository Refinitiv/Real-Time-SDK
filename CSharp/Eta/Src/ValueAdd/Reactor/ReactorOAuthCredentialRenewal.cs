/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using System.Diagnostics;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This class represents the OAuth credential renewal information.
    /// </summary>
    sealed public class ReactorOAuthCredentialRenewal
    {
        private Buffer m_ClientId = new Buffer();
        private Buffer m_ClientSecret = new Buffer();
        private Buffer m_TokenScope = new Buffer();
        private Buffer m_ClientJwk = new Buffer();
        private Buffer m_Audience = new Buffer();

        /// <summary>
        /// Creates <see cref="ReactorOAuthCredentialRenewal"/>
        /// </summary>
        public ReactorOAuthCredentialRenewal()
        {
            Clear();
        }

        /// <summary>
        /// Clears all values to default
        /// </summary>
        public void Clear()
        {
            m_ClientId.Clear();
            m_ClientSecret.Clear();
            m_TokenScope.Clear();
            m_ClientJwk.Clear();
            m_Audience.Clear();
        }

        /// <summary>
        /// Gets or sets the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
        /// </summary>
        public Buffer ClientId
        {
            get { return m_ClientId; }
            
            set { m_ClientId.Copy(value); }
        }

        /// <summary>
        /// Gets the clientSecret, also known as the Service Account password, used to authenticate with RDP token service. Mandatory for V2 Client Credentials Logins and used in conjunction with clientID.
        /// </summary>
        public Buffer ClientSecret
        {
            get { return m_ClientSecret; }

            set { m_ClientSecret.Copy(value); }
        }

        /// <summary>
        /// Gets or sets the token scope to limit the scope of generated token. Optional.
        /// </summary>
        public Buffer TokenScope
        {
            get { return m_TokenScope; }

            set { m_TokenScope.Copy(value); }
        }

        /// <summary>
        /// Gets or sets the JWK formatted private key used to create the JWT. The JWT is used to authenticate with the RDP token service. Mandatory for V2 logins with client JWT logins 
        /// </summary>
        public Buffer ClientJwk
        {
            get { return m_ClientJwk; }

            set { ClientJwk.Copy(value); }
        }

        /// <summary>
        /// Gets or sets the audience claim for the JWT.
        /// </summary>
        public Buffer Audience
        {
            get { return m_Audience; }

            set { Audience.Copy(value); }
        }

        /// <summary>
        /// Performs a deep copy of <see cref="ReactorOAuthCredentialRenewal"/> object.
        /// </summary>
        /// <param name="destReactorOAuthCredentialRenewal">To copy OAuth credential renewal object into.
        /// It cannot be null.</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating success or failure of copy operations.</returns>
        public ReactorReturnCode Copy(ReactorOAuthCredentialRenewal destReactorOAuthCredentialRenewal)
        {
            Debug.Assert(destReactorOAuthCredentialRenewal is not null, "destReactorOAuthCredentialRenewal must be non-null");

            if (destReactorOAuthCredentialRenewal is null)
                return ReactorReturnCode.FAILURE;

            if(m_ClientId.Length != 0)
            {
                ByteBuffer byteBuffer = new ByteBuffer(m_ClientId.Length);
                m_ClientId.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredentialRenewal.ClientId.Data(byteBuffer);
            }

            if (m_ClientSecret.Length != 0)
            {
                ByteBuffer byteBuffer = new ByteBuffer(m_ClientSecret.Length);
                m_ClientSecret.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredentialRenewal.ClientSecret.Data(byteBuffer);
            }

            if (m_TokenScope.Length != 0)
            {
                ByteBuffer byteBuffer = new ByteBuffer(m_TokenScope.Length);
                m_TokenScope.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredentialRenewal.TokenScope.Data(byteBuffer);
            }

            if (m_ClientJwk.Length != 0)
            {
                ByteBuffer byteBuffer = new ByteBuffer(m_ClientJwk.Length);
                m_ClientJwk.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredentialRenewal.ClientJwk.Data(byteBuffer);
            }

            if (m_Audience.Length != 0)
            {
                ByteBuffer byteBuffer = new ByteBuffer(m_Audience.Length);
                m_Audience.Copy(byteBuffer);
                byteBuffer.Flip();
                destReactorOAuthCredentialRenewal.Audience.Data(byteBuffer);
            }

            return ReactorReturnCode.SUCCESS;
        }
    }
}