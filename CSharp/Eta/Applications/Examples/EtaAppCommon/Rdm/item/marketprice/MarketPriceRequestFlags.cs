/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Example.Common
{
    [Flags]
    public enum MarketPriceRequestFlags
    {
        NONE = 0,
        HAS_QOS = 0x001,
        HAS_PRIORITY = 0x002,
        HAS_SERVICE_ID = 0x004,
        HAS_WORST_QOS = 0x008,
        HAS_VIEW = 0x010,
        STREAMING = 0x020,
        PRIVATE_STREAM = 0x040
    }
}
