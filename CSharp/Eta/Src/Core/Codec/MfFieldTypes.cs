/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// Enumerated Marketfeed types.
	/// </summary>
	sealed public class MfFieldTypes
	{
		// MfFieldTypes class cannot be instantiated
		private MfFieldTypes()
		{
			throw new System.NotImplementedException();
		}

        /// <summary>
        /// NONE
        /// </summary>
		public const int NONE = -1;

        /// <summary>
        /// TIME_SECONDS
        /// </summary>
		public const int TIME_SECONDS = 0;

        /// <summary>
        /// INTEGER
        /// </summary>
		public const int INTEGER = 1;

        /// <summary>
        /// DATE
        /// </summary>
		public const int DATE = 3;

        /// <summary>
        /// PRICE
        /// </summary>
		public const int PRICE = 4;

        /// <summary>
        /// ALPHANUMERIC
        /// </summary>
		public const int ALPHANUMERIC = 5;

        /// <summary>
        /// ENUMERATED
        /// </summary>
		public const int ENUMERATED = 6;

        /// <summary>
        /// TIME
        /// </summary>
		public const int TIME = 7;

        /// <summary>
        /// BINARY
        /// </summary>
		public const int BINARY = 8;
	}

}