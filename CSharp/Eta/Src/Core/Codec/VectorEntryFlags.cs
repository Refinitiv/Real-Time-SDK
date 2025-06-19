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
    /// 
    /// <summary>
    /// Combination of bit values that indicate whether optional <see cref="VectorEntry"/> content is present.
    /// </summary>
    /// <seealso cref="VectorEntry"/>
    [Flags]
    public enum VectorEntryFlags
	{
		/// <summary>
		/// (0x00) No Vector Entry Flags </summary>
		NONE = 0x00,

		/// <summary>
		/// (0x01) Indicates the presence of the Permission Expression in this
		/// container entry and indicates authorization information for this entry.
		/// </summary>
		HAS_PERM_DATA = 0x01
	}

}