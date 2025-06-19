/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{

	/// <summary>
	/// A single defined enumerated value.
	/// </summary>
	public interface IEnumType
	{
        /// <summary>
        /// The actual value representing the type.
        /// </summary>
        /// <value> the value </value>
        ushort Value { get; }

		/// <summary>
		/// A brief string representation describing what the type means (For example,
		/// this may be an abbreviation of a currency to be displayed to a user).
		/// </summary>
		/// <value> the display </value>
		Buffer Display { get; }

		/// <summary>
		/// A more elaborate description of what the value means. This information is
		/// typically optional and not displayed.
		/// </summary>
		/// <value> the meaning </value>
		Buffer Meaning { get; }
	}
}