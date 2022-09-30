/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Codec
{
	/// <summary>
	/// Provides information on the health of the data flowing within a stream.
	/// </summary>
	/// <seealso cref="State"/>
	sealed public class DataStates
	{
		// DataStates class cannot be instantiated
		private DataStates()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// Indicates there is no change in the current state of the data. When
		/// available, it is preferable to send more concrete state information (such
		/// as OK or SUSPECT) instead of NO_CHANGE. This typically conveys code or
		/// text information associated with an item group, but no change to the
		/// group's previous data and stream state has occurred.
		/// </summary>
		public const int NO_CHANGE = 0;

		/// <summary>
		/// Data is Ok. All data associated with the stream is healthy and current
		/// </summary>
		public const int OK = 1;

		/// <summary>
		/// Data is Suspect. Similar to a stale data state, indicates that the health
		/// of some or all data associated with the stream is out of date or cannot
		/// be confirmed that it is current.
		/// </summary>
		public const int SUSPECT = 2;

		/// <summary>
		/// Provides string representation for a data state value.
		/// </summary>
		/// <param name="dataState"> <seealso cref="DataStates"/> enumeration to convert to string
		/// </param>
		/// <returns> string representation for a data state value </returns>
		public static string ToString(int dataState)
		{
			switch (dataState)
			{
				case NO_CHANGE:
					return "NO_CHANGE";
				case OK:
					return "OK";
				case SUSPECT:
					return "SUSPECT";
				default:
					return "Unknown Data State";
			}
		}

		/// <summary>
		/// Provides string representation of meaning associated with data state.
		/// </summary>
		/// <param name="dataState"> <seealso cref="DataStates"/> enumeration to get info for
		/// </param>
		/// <returns> string representation of meaning associated with data state </returns>
		public static string Info(int dataState)
		{
			switch (dataState)
			{
				case DataStates.NO_CHANGE:
					return "No Change";
				case DataStates.OK:
					return "Ok";
				case DataStates.SUSPECT:
					return "Suspect";
				default:
					return "Unknown Data State";
			}
		}
	}

}