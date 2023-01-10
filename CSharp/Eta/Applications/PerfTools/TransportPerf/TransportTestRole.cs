/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Transport performance test role.
    /// </summary>
    public enum TransportTestRole
    {
        UNINIT = 0x00,
        WRITER = 0x01,
        READER = 0x02,
        REFLECTOR = 0x04
    }
}
