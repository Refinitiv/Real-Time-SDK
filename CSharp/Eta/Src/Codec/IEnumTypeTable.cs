/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

namespace Refinitiv.Eta.Codec
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
        /// <returns> the maxValue </returns>
        UInt16 MaxValue { get; }

		/// <summary>
		/// The list of enumerated types.
		/// </summary>
		/// <returns> the enumTypes </returns>
		List<IEnumType> EnumTypes { get; }

        /// <summary>
        /// A list of fieldId's representing fields that reference this table.
        /// </summary>
        /// <returns> fidReferences </returns>
        List<Int16> FidReferences { get; }
	}
}