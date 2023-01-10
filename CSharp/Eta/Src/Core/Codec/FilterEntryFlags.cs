/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// A combination of bit values that indicate the presence of optional <see cref="FilterEntry"/> content.
    /// </summary>
    /// <seealso cref="FilterEntry"/>
    [Flags]
    public enum FilterEntryFlags
	{
		/// <summary>
		/// (0x00) No filter item flags </summary>
		NONE = 0x00,

		/// <summary>
		/// (0x01) Indicates the presence of permData in this container entry and
		/// indicates authorization information for this entry.
		/// </summary>
		HAS_PERM_DATA = 0x01,

		/// <summary>
		/// (0x02) Indicates the presence of containerType in this entry. This flag
		/// is used when the entry differs from the specified
		/// <see cref="FilterList.ContainerType"/>.
		/// </summary>
		HAS_CONTAINER_TYPE = 0x02
	}

}