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
    /// Combination of bit values that indicate whether optional <see cref="Vector"/> content is present.
    /// </summary>
    /// <seealso cref="Vector"/>
    [Flags]
    public enum VectorFlags
	{
		/// <summary>
		/// (0x00) No flags </summary>
		NONE = 0x00,

		/// <summary>
		/// (0x01) Indicates that the <see cref="Vector"/> contains local set definition
		/// information. Local set definitions correspond to data contained in this
		/// <see cref="Vector"/>'s entries and are used for encoding or decoding.
		/// </summary>
		HAS_SET_DEFS = 0x01,

		/// <summary>
		/// (0x02) Indicates that the <see cref="Vector"/> contains summary data.
		/// <ul>
		/// <li>If this flag is set while encoding, summary data must be provided by
		/// encoding or populating encodedSummaryData with pre-encoded data.</li>
		/// <li>If this flag is set while decoding, summary data is contained as part
		/// of <see cref="Vector"/> and the user can choose whether to decode it.</li>
		/// </ul>
		/// </summary>
		HAS_SUMMARY_DATA = 0x02,

		/// <summary>
		/// (0x04) Indicates that permission information is included with some vector
		/// entries. The <see cref="Vector"/> encoding functionality sets this flag value
		/// on the user's behalf if an entry is encoded with its own permData. A
		/// decoding application can check this flag to determine whether a contained
		/// entry has permData and is often useful for fan out devices (if an entry
		/// does not have permData, the fan out device can likely pass on data and
		/// not worry about special permissioning for the entry). Each entry also
		/// indicates the presence of permission data via the use of
		/// <see cref="VectorEntryFlags.HAS_PERM_DATA"/>.
		/// </summary>
		HAS_PER_ENTRY_PERM_DATA = 0x04,

		/// <summary>
		/// (0x08) Indicates that the totalCountHint member is present.
		/// totalCountHint can provide an approximation of the total number of
		/// entries sent across all vectors on all parts of the refresh message. Such
		/// information is useful in determining the amount of resources to allocate
		/// for caching or displaying all expected entries.
		/// </summary>
		HAS_TOTAL_COUNT_HINT = 0x08,

		/// <summary>
		/// (0x10) Indicates that the <see cref="Vector"/> may leverage sortable action
		/// types. If a <see cref="Vector"/> is sortable, all components must properly
		/// handle changing index values based on insert and delete actions. If a
		/// component does not properly handle these action types, it can result in
		/// the corruption of the <see cref="Vector"/>'s contents.
		/// </summary>
		SUPPORTS_SORTING = 0x10
	}

}