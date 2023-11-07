/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;

/// <summary>
/// ServiceEndpointDiscoveryOption is used to specify query options for <see cref="ServiceEndpointDiscovery.RegisterClient(LSEG.Ema.Access.ServiceEndpointDiscoveryOption, LSEG.Ema.Access.IServiceEndpointDiscoveryClient, object?)"/>}
/// </summary>
public sealed class ServiceEndpointDiscoveryOption
{
    /// <summary>
    /// Specifies optionally an audience string used with JWT authentication.
    /// </summary>
    public string? Audience { get; set; }

    /// <summary>
    /// Specifies optionally a client JWK string used to authenticate to the Authorization Server.
    /// </summary>
    public string? ClientJWK { get; set; }

    /// <summary>
    /// Specifies optionally a client secret used by OAuth client to authenticate to the Authorization Server.
    /// </summary>
    public string? ClientSecret { get; set; }

    /// <summary>
    /// Specifies the proxy password to authenticate.
    /// </summary>
    public string? ProxyPassword { get; set; }

    /// <summary>
    /// Specifies the proxy user name to authenticate.
    /// </summary>
    public string? ProxyUserName { get; set; }

    /// <summary>
    /// Specifies the port number of the HTTP proxy server.
    /// </summary>
    public string? ProxyPort { get; set; }

    /// <summary>
    /// Specifies the address or hostname of the HTTP proxy server.
    /// </summary>
    public string? ProxyHostName { get; set; }

    /// <summary>
    /// Specifies a data format protocol to get endpoints according to the protocol.
    /// </summary>
    public DataformatProtocol DataFormat { get; set; }

    /// <summary>
    /// Specifies a transport protocol to get endpoints according to the protocol.<br/>
    /// This is an optional option to limit number of endpoints.
    /// </summary>
    public TransportProtocol Transport { get; set; }

    /// <summary>
    /// Specifies the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
    /// </summary>
    public string? ClientId { get; set; }

    /// <summary>
    /// Clears the ServiceEndpointDiscoveryOption and sets all the defaults.
    /// </summary>
    public void Clear()
    {
        Audience = null;
        ClientJWK = null;
        ClientSecret = null;
        ProxyPassword = null;
        ProxyUserName = null;
        ProxyPort = null;
        ProxyHostName = null;
        ProxyPort = null;
        ProxyHostName = null;
    }
}