/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Rdm
{
	/// <summary>
	/// RDM Instrument Name Types.
	/// 
	/// </summary>
	sealed public class InstrumentNameTypes
	{
		// InstrumentNameTypes class cannot be instantiated
		private InstrumentNameTypes()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// Symbology is not specified or not applicable </summary>
		public const int UNSPECIFIED = 0;
		/// <summary>
		/// Instrument Code </summary>
		public const int RIC = 1;
		/// <summary>
		/// Contributor </summary>
		public const int CONTRIBUTOR = 2;
		/* Maximum reserved Quote Symbology */
		internal const int MAX_RESERVED = 127;
	}

}
