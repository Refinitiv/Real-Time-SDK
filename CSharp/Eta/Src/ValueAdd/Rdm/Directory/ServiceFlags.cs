/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    [Flags]
    public enum ServiceFlags : int
    {
        NONE = 0x00,
        
        HAS_INFO = 0x01,
        
        HAS_STATE = 0x02,

        HAS_LOAD = 0x04,

        HAS_DATA = 0x08,

        HAS_LINK = 0x10
    }
}
