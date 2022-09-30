/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace Refinitiv.Eta.Codec
{
    /// <summary>
    /// Combination of bit values that indicates the presence of optional field list content.
    /// </summary>
    /// <seealso cref="FieldList"/>
    [Flags]
    public enum FieldListFlags
	{
		/// <summary>
		/// (0x00) None of the optional flags are set </summary>
		NONE = 0x00,

		/// <summary>
		/// (0x01) Indicates that dictionaryId and fieldListNum members are present,
		/// which should be provided as part of the initial refresh message on a
		/// stream or on the first refresh message after issuance of a CLEAR_CACHE command.
		/// </summary>
		HAS_FIELD_LIST_INFO = 0x01,

		/// <summary>
		/// (0x02) Indicates that the <see cref="FieldList"/> contains set-defined data.
		/// This value can be set in addition to <see cref="HAS_STANDARD_DATA"/> if both
		/// standard and set-defined data are present in this <see cref="FieldList"/>. If
		/// no entries are present in the <see cref="FieldList"/>, this flag value should not be set.
		/// </summary>
		HAS_SET_DATA = 0x02,

		/// <summary>
		/// (0x04) Indicates the presence of a setId, used to determine the set
		/// definition used for encoding or decoding the set data on this <seealso cref="FieldList"/>.
		/// </summary>
		HAS_SET_ID = 0x04,

		/// <summary>
		/// (0x08) Indicates that the <see cref="FieldList"/> contains standard
		/// fieldId-value pair encoded data. This value can be set in addition to
		/// <see cref="HAS_SET_DATA"/> if both standard and set-defined data are present
		/// in this <see cref="FieldList"/>. If no entries are present in the
		/// <see cref="FieldList"/>, this flag value should not be set.
		/// </summary>
		HAS_STANDARD_DATA = 0x08,
	}

}