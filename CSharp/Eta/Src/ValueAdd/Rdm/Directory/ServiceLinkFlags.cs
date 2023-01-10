/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Service Info Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="ServiceLink"/>
    [Flags]
    public enum ServiceLinkFlags : int
    {
        /// <summary>
        /// (0x00) No flags set. 
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// Indicates presence of the source type. 
        /// </summary>
        HAS_TYPE = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of the link code. 
        /// </summary>
        HAS_CODE = 0x02,

        /// <summary>
        /// (0x04) Indicates presence of link text.
        /// </summary>
        HAS_TEXT = 0x04,
    }
}
