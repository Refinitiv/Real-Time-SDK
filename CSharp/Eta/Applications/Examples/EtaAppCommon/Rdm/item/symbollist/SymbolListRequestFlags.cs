/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Example.Common
{
    public enum SymbolListRequestFlags
    {
        // (0x00) No flags set.
        NONE = 0x0000,

        // (0x0001) Indicates presence of the qos member.
        HAS_QOS = 0x0001,

        // (0x0002) Indicates presence of the qos member.
        HAS_PRIORITY = 0x0002,

        // (0x0004) Indicates presence of the service id member.
        HAS_SERVICE_ID = 0x0004,

        // (0x0008) Indicates presence of the streaming flag.
        STREAMING = 0x0008
    }
}
