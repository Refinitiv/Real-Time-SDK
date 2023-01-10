/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// A table of enumerated types.  A field that uses this table will contain a value
	/// corresponding to an enumerated type in this table.
	/// </summary>
	public interface IEnumTypeTable
	{
        /// <summary>
        /// The highest value type present. This also indicates the length of the enumTypes list.
        /// </summary>
        /// <value> the maxValue </value>
        UInt16 MaxValue { get; }

		/// <summary>
		/// The list of enumerated types.
		/// </summary>
		/// <value> the enumTypes </value>
		List<IEnumType> EnumTypes { get; }

        /// <summary>
        /// A list of fieldId's representing fields that reference this table.
        /// </summary>
        /// <value> fidReferences </value>
        List<Int16> FidReferences { get; }
	}
}