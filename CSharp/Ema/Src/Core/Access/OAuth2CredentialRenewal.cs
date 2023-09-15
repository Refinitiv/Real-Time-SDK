/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access;

/// <summary>
/// Representation of the OAuth credential renewal information.
/// </summary>
public class OAuth2CredentialRenewal
{
    internal ReactorOAuthCredentialRenewal m_Credentials = new();

    /// <summary>
    /// Clears the OAuth2CredentialRenewal and sets all the defaults.
    /// Invoking Clear() method clears all the values and resets all the defaults.
    /// </summary>
    ///
    /// <returns>reference to this object</returns>
    public OAuth2CredentialRenewal Clear()
    {
        m_Credentials.Clear();
        return this;
    }

    /// <summary>
    /// Sets the clientID used for RDP token service. Mandatory, used to specify
    /// Application ID obtained from App Generator for V1 oAuth Password Credentials, or
    /// to specify Service Account username for V2 Client Credentials and V2 Client
    /// Credentials with JWT Logins.
    /// </summary>
    ///
    /// <param name="clientId">the unique identifier for the application</param>
    ///
    /// <returns>reference to this object</returns>
    public OAuth2CredentialRenewal ClientId(string clientId)
    {
        m_Credentials.ClientId.Data(clientId);
        return this;
    }

    /// <summary>
    /// Returns the current clientID used for RDP token service.
    /// </summary>
    public string ClientId()
    {
        return m_Credentials.ClientId.ToString();
    }

    /// <summary>
    /// Sets the clientSecret, also known as the Service Account password, used to
    /// authenticate with RDP token service. Mandatory for V2 Client Credentials Logins
    /// and used in conjunction with clientID.
    /// </summary>
    ///
    /// <param name="clientSecret">the client secret</param>
    ///
    /// <returns>reference to this object</returns>
    public OAuth2CredentialRenewal ClientSecret(string clientSecret)
    {
        m_Credentials.ClientSecret.Data(clientSecret);
        return this;
    }

    /// <summary>
    /// Returns the current clientSecret, also known as the Service Account password.
    /// </summary>
    public string ClientSecret()
    {
        return m_Credentials.ClientSecret.ToString();
    }

    /// <summary>
    /// Sets the JWK formatted private key used to create the JWT. The JWT is used to
    /// authenticate with the RDP token service. Mandatory for V2 logins with client JWT
    /// logins
    /// </summary>
    ///
    /// <param name="clientJwk">the client JWK string, encoded in  JSON format.</param>
    ///
    /// <returns>reference to this object</returns>
    public OAuth2CredentialRenewal ClientJWK(string clientJwk)
    {
        m_Credentials.ClientJwk.Data(clientJwk);
        return this;
    }

    /// <summary>
    /// Returns the current JWK used to create the JWT.
    /// </summary>
    public string ClientJWK()
    {
        return m_Credentials.ClientJwk.ToString();
    }

    /// <summary>
    /// Sets the token scope to limit the scope of generated token. Optional.
    /// </summary>
    ///
    /// <param name="tokenScope">the token scope</param>
    ///
    /// <returns>reference to this object</returns>
    public OAuth2CredentialRenewal TokenScope(string tokenScope)
    {
        m_Credentials.TokenScope.Data(tokenScope);
        return this;
    }

    /// <summary>
    /// Returns the current token scope.
    /// </summary>
    public string TokenScope()
    {
        return m_Credentials.TokenScope.ToString();
    }

    /// <summary>
    /// Specifies the audience claim for the JWT. Optional and only used for V2 Client
    /// Credentials with JWT.
    /// </summary>
    ///
    /// <param name="audience">the audience string</param>
    ///
    /// <returns>reference to this object</returns>
    public OAuth2CredentialRenewal Audience(string audience)
    {
        m_Credentials.Audience.Data(audience);
        return this;
    }

    /// <summary>
    /// Returns the current audience claim for the JWT.
    /// </summary>
    public string Audience()
    {
        return m_Credentials.Audience.ToString();
    }

}
