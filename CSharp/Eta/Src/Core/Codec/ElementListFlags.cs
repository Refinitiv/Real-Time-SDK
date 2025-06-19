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
    /// Combination of bit values that indicate whether optional, element-list content is present.
    /// </summary>
    /// <seealso cref="ElementList"/>
    [Flags]
    public enum ElementListFlags
	{
		/// <summary>
		/// (0x00) None of the optional flags are set. </summary>
		NONE = 0x00,

		/// <summary>
		/// (0x01) Indicates the presence of has element list number.
		/// This member is provided as part of the initial refresh message on a stream or on the
		/// first refresh message after a CLEAR_CACHE command.
		/// </summary>
		HAS_ELEMENT_LIST_INFO = 0x01,

        /// <summary>
        /// (0x02) Indicates that <see cref="ElementList"/> contains set-defined data.
        /// If both standard and set-defined data are present in this
        /// <see cref="ElementList"/>, this value can be set in addition to <see cref="ElementListFlags.HAS_STANDARD_DATA"/>.
        /// If no entries are present in the <see cref="ElementList"/>, do not set this flag value.
        /// </summary>
        HAS_SET_DATA = 0x02,

		/// <summary>
		/// (0x04) Indicates the presence of a setId and determines the set
		/// definition to use when encoding or decoding set data on this <see cref="ElementList"/>.
		/// </summary>
		HAS_SET_ID = 0x04,

        /// <summary>
        /// (0x08) Indicates that the <see cref="ElementList"/> contains standard element
        /// name, dataType, value-encoded data. You can set this value in addition to
        /// <see cref="ElementListFlags.HAS_SET_DATA"/> if both standard and set-defined data are present
        /// in this <see cref="ElementList"/>. If no entries are present in the
        /// <see cref="ElementList"/>, do not set this flag value.
        /// </summary>
        HAS_STANDARD_DATA = 0x08,
	}

}