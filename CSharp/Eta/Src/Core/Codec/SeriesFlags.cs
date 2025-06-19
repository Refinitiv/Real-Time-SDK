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
    /// A combination of bit values that indicates the presence of optional <see cref="Series"/> content.
    /// </summary>
    /// <seealso cref="Series"/>
    [Flags]
    public enum SeriesFlags
	{

		/// <summary>
		/// (0x00) None of the optional flags are present. </summary>
		NONE = 0x00,

		/// <summary>
		/// (0x01) Indicates that the <see cref="Series"/> contains local set definition
		/// information. Local set definitions correspond to data contained in this
		/// Series's entries and encode or decode their contents.
		/// </summary>
		HAS_SET_DEFS = 0x01,

		/// <summary>
		/// (0x02) Indicates that the <see cref="Series"/> contains summary data.
		/// <ul>
		/// <li>If set while encoding, summary data must be provided by encoding or
		/// populating encodedSummaryData with pre-encoded information.</li>
		/// <li>If set while decoding, summary data is contained as part of
		/// <see cref="Series"/> and the user can choose to decode it.</li>
		/// </ul>
		/// </summary>
		HAS_SUMMARY_DATA = 0x02,

		/// <summary>
		/// (0x04) Indicates the presence of the totalCountHint member, which can
		/// provide an approximation of the total number of entries sent across maps
		/// on all parts of the refresh message. Such information is useful when
		/// determining resource allocation for caching or displaying all expected entries.
		/// </summary>
		HAS_TOTAL_COUNT_HINT = 0x04
	}

}