/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{
    sealed internal class RwfDataConstants
	{
		// RwfDataConstants class cannot be instantiated
		private RwfDataConstants()
		{
		}

		public const byte MAJOR_VERSION_1 = 14;
		public const byte MINOR_VERSION_1 = 1;
	}

}