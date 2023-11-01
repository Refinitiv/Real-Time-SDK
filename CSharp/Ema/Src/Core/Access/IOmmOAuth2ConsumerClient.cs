/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;

/// <summary>
/// Application-provided callback for OAuth2 credential renewal.
/// </summary>
public interface IOmmOAuth2ConsumerClient
{
    /// <summary>
    /// Invoked when new credentials are required
    /// </summary>
    /// <param name="evt">Renewal event information</param>
    /// <param name="creds">Used to specify which credentials need renewal</param>
    public void OnOAuth2CredentialRenewal(IOmmConsumerEvent evt, OAuth2CredentialRenewal creds);
}
