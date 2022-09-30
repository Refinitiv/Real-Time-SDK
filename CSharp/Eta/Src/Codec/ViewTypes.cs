/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Rdm
{
    /// <summary>
    /// RDM View Types.
    /// </summary>
    sealed public class ViewTypes
	{
		// ViewTypes class cannot be instantiated
		private ViewTypes()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// View Data contains a list of Field IDs </summary>
		public const int FIELD_ID_LIST = 1;
		/// <summary>
		/// View Data contains a list of Element Names </summary>
		public const int ELEMENT_NAME_LIST = 2;
	}

}