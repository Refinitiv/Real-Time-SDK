/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Types indicating the dataformat query parameter.
    /// <see cref="ReactorServiceDiscoveryOptions"/>
    /// </summary>
    public enum ReactorDiscoveryDataFormatProtocol : int
    {
        /// <summary>
        /// Unknown data format
        /// </summary>
        RD_DP_INIT = 0,

        /// <summary>
        /// Rwf data format protocol
        /// </summary>
        RD_DP_RWF = 1,

        /// <summary>
        /// JSON2 data format protocol 
        /// </summary>
        RD_DP_JSON2 = 2,
    }
}
