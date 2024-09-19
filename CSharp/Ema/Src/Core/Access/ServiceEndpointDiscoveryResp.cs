/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Text;
namespace LSEG.Ema.Access;

/// <summary>
/// ServiceEndpointDiscoveryResp represents a response from RDP service discovery which contains a list of ServiceEndpointDiscoveryInfo
/// </summary>
public sealed class ServiceEndpointDiscoveryResp
{
    private readonly StringBuilder _strBuilder = new(1024);

    /// <summary>
    /// Gets a list of ServiceEndpointDiscoveryInfo of this ServiceEndpointDiscoveryResp.
    /// </summary>
    public List<ServiceEndpointDiscoveryInfo> ServiceEndpointInfoList { get; } = new List<ServiceEndpointDiscoveryInfo>();

    /// <summary>
    /// Gets string representation of this object.
    /// </summary>
    /// <returns>string representation of ServiceEndpointDiscoveryInfo.</returns>
    public override string ToString()
    {
        _strBuilder.Length = 0;

        Utilities.AddIndent(_strBuilder.Append("Services : \n"), 1);
        for (int index = 0; index < ServiceEndpointInfoList.Count; index++)
        {
            Utilities.AddIndent(_strBuilder.Append(ServiceEndpointInfoList[index].ToString(2))
                    .Append('\n'), 1);
        }

        return _strBuilder.ToString();
    }
}
