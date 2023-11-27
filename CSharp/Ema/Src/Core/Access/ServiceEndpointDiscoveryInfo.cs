/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Text;

namespace LSEG.Ema.Access;

/// <summary>
/// ServiceEndpointDiscoveryInfo represents an service endpoint information from RDP service discovery.<br/>
/// This class is retrieved from ServiceEndpointDiscoveryResp.
/// </summary>
/// <see cref="ServiceEndpointDiscoveryResp"/>
public sealed class ServiceEndpointDiscoveryInfo
{
    /// <summary>
    /// Gets a list of data format supported by this endpoint.
    /// </summary>
    public List<string> DataFormatList { get; } = new(2);

    /// <summary>
    /// Gets an endpoint or domain name for establishing a connection.
    /// </summary>
    public string? Endpoint { get; set; }

    /// <summary>
    /// Gets a list of locations where the infrastructure is deployed in Refinitiv Real-Time - Optimized.
    /// </summary>
    public List<string> LocationList { get; } = new(2);

    /// <summary>
    /// Gets a port for establishing a connection.
    /// </summary>
    public string? Port { get; set; }

    /// <summary>
    /// Gets a public Refinitiv Real-Time - Optimized provider
    /// </summary>
    public string? Provider { get; set; }

    /// <summary>
    /// Gets a transport type
    /// </summary>
    public string? Transport { get; set; }

    /// <summary>
    /// Gets string representation of this object.
    /// </summary>
    /// <param name="indent">indent</param>
    /// <returns>string representation of ServiceEndpointDiscoveryInfo.</returns>
    public string ToString(int indent)
    {
        StringBuilder _strBuilder = new(64);

        Utilities.AddIndent(_strBuilder.Append("Service : \n"), indent);
        Utilities.AddIndent(_strBuilder.Append("Provider : ").Append(Provider).Append("\n"), indent);
        Utilities.AddIndent(_strBuilder.Append("Transport : ").Append(Transport).Append("\n"), indent);
        Utilities.AddIndent(_strBuilder.Append("Endpoint : ").Append(Endpoint).Append("\n"), indent);
        Utilities.AddIndent(_strBuilder.Append("Port : ").Append(Port).Append("\n"), indent);

        _strBuilder.Append("Data Format : ");
        for (int index = 0; index < DataFormatList.Count; index++)
            _strBuilder.Append(DataFormatList[index]).Append("  ");

        Utilities.AddIndent(_strBuilder.Append("\n"), indent);
        _strBuilder.Append("Location : ");
        for (int index = 0; index < LocationList.Count; index++)
            _strBuilder.Append(LocationList[index]).Append("  ");

        return _strBuilder.ToString();
    }
}