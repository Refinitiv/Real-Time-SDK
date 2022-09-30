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
    public enum ServiceLinkFlags : int
    {
        // (0x00) No flags set. 
        NONE = 0x00,

        // Indicates presence of the source type. 
        HAS_TYPE = 0x01,

        // (0x02) Indicates presence of the link code. 
        HAS_CODE = 0x02,

        // (0x04) Indicates presence of link text.
        HAS_TEXT = 0x04,
    }
}
