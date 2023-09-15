/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;

/// <summary>
/// DataformatProtocol represents data format protocol options.
/// </summary>
public enum DataformatProtocol
{
    /// <summary>
    /// Indicates undefined data format protocol
    /// </summary>
    UNKNOWN = 0,

    /// <summary>
    /// Indicates RWF data format protocol
    /// </summary>
    RWF = 1,

    /// <summary>
    /// Indicates tr_json2 data format protocol
    /// </summary>
    JSON2 = 2
}
