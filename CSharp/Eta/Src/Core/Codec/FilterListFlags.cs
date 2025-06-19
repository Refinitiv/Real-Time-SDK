/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// A combination of bit values to indicate presence of optional
    /// <see cref="FilterList"/> content.
    /// </summary>
    /// <seealso cref="FilterList"/>
    [Flags]
    public enum FilterListFlags
	{
		/// <summary>
		/// (0x00) None of the optional flags are set. </summary>
		NONE = 0x00,

		/// <summary>
		/// (0x01) Indicates that the Filter Entries in the Filter List contain Permission Data.
		/// </summary>
		HAS_PER_ENTRY_PERM_DATA = 0x01,

		/// <summary>
		/// (0x02) Indicates that the Filter List contains a Total Count Hint. </summary>
		HAS_TOTAL_COUNT_HINT = 0x02
	}

}