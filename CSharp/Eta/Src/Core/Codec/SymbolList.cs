/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.Rdm
{
    /// <summary>
    /// The Symbolist definitions for RDM
    /// </summary>
    sealed public class SymbolList
	{
		private SymbolList()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// SymbolListSupportFlags
		/// </summary>
		public class SymbolListSupportFlags
		{
			// SymbolListSupportFlags class cannot be instantiated
			private SymbolListSupportFlags()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Only symbol list name </summary>
			public const int SYMBOL_LIST_SUPPORT_NAMES_ONLY = 0;
			/// <summary>
			///  Enhanced symbol list data </summary>
			public const int SYMBOL_LIST_SUPPORT_DATA_STREAM = 1;
		}

		/// <summary>
		/// Symbol List request behavior flags.
		/// </summary>
		public class SymbolListDataStreamRequestFlags
		{
			// SymbolListDataStreamRequestFlags class cannot be instantiated
			private SymbolListDataStreamRequestFlags()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			///* Only Symbol List Name </summary>
			public const int SYMBOL_LIST_NAMES_ONLY = 0x00000000;
			/// <summary>
			/// Enhanced Symbol List Data Stream </summary>
			public const int SYMBOL_LIST_DATA_STREAMS = 0x00000001;
			/// <summary>
			/// Enhanced Symbol List Snapshots </summary>
			public const int SYMBOL_LIST_DATA_SNAPSHOTS = 0x00000002;
		}

		/// <summary>
		/// Element names for SymbolList.
		/// </summary>
		public class ElementNames
		{
			// ElementNames class cannot be instantiated
			private ElementNames()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// :SymbolListData Behavior </summary>
			public static readonly Buffer SYMBOL_LIST_BEHAVIORS = new Buffer();
			/// <summary>
			/// :SymbolListDataStream </summary>
			public static readonly Buffer SYMBOL_LIST_DATA_STREAMS = new Buffer();

			static ElementNames()
			{
				// :SymbolList Behavior
				SYMBOL_LIST_BEHAVIORS.Data(":SymbolListBehaviors");
				
				// :SymbolListDataStream
				SYMBOL_LIST_DATA_STREAMS.Data(":DataStreams");
			}
		}
	}

}