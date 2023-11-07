/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;

/// <summary>
/// ServiceEndpointDiscoveryEvent encapsulates query identifiers.
/// </summary>
public class ServiceEndpointDiscoveryEvent
{
    /// <summary>
    /// Returns an identifier (a.k.a., closure) associated with a query by consumer application.
    /// Application associates the closure with a query ServiceEndpointDiscovery.
    /// </summary>
    /// <seealso cref="ServiceEndpointDiscovery"/>
	public object? Closure { get; set; }

    /// <summary>
    /// Returns ServiceEndpointDiscovery instance for this event.
    /// </summary>
    public ServiceEndpointDiscovery? ServiceEndpointDiscovery { get; set; }
}