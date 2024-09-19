/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// The Ack Message nakCodes, used to indicate a reason for a negative acknowledgment.
	/// </summary>
	/// <seealso cref="IAckMsg"/>
	sealed public class NakCodes
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private NakCodes()
		{
            throw new NotImplementedException();
		}

		/// <summary>
		/// No Nak Code </summary>
		public const int NONE = 0;

		/// <summary>
		/// Access Denied (user not properly permissioned for posting on the item or service)
		/// </summary>
		public const int ACCESS_DENIED = 1;

		/// <summary>
		/// Denied by source (source being posted to has denied accepting this post message)
		/// </summary>
		public const int DENIED_BY_SRC = 2;

		/// <summary>
		/// Source Down (source being posted to is down or not available) </summary>
		public const int SOURCE_DOWN = 3;

		/// <summary>
		/// Source Unknown (source being posted to is unknown and is unreachable) </summary>
		public const int SOURCE_UNKNOWN = 4;

		/// <summary>
		/// No Resources (some component along the path of the post message does not
		/// have appropriate resources available to continue processing the post)
		/// </summary>
		public const int NO_RESOURCES = 5;

		/// <summary>
		/// No Response (may mean that the source is unavailable or there is a delay
		/// in processing the posted information)
		/// </summary>
		public const int NO_RESPONSE = 6;

		/// <summary>
		/// Gateway is down (gateway device for handling posted or contributed
		/// information is down or not available)
		/// </summary>
		public const int GATEWAY_DOWN = 7;

		/* Reserved */
		internal const int RESERVED8 = 8;
		/* Reserved */
		internal const int RESERVED9 = 9;

		/// <summary>
		/// Unknown Symbol (item information provided with the post message is not
		/// recognized by the system)
		/// </summary>
		public const int SYMBOL_UNKNOWN = 10;

		/// <summary>
		/// Item not open (item being posted to does not have an available stream open)
		/// </summary>
		public const int NOT_OPEN = 11;

		/// <summary>
		/// Nak being sent due to invalid content (content of the post message is
		/// invalid and cannot be posted, it does not match the expected formatting for this post)
		/// </summary>
		public const int INVALID_CONTENT = 12;

        /// <summary>
        /// Converts nak code to string value
        /// </summary>
        /// <param name="nakCode">The nak code</param>
        /// <returns>The string representation of the nak code
        /// </returns>
		public static string ToString(int nakCode)
		{
			string ret = "";

			switch (nakCode)
			{
				case NONE:
					ret = "NONE";
					break;
				case ACCESS_DENIED:
					ret = "ACCESS_DENIED";
					break;
				case DENIED_BY_SRC:
					ret = "DENIED_BY_SRC";
					break;
				case SOURCE_DOWN:
					ret = "SOURCE_DOWN";
					break;
				case SOURCE_UNKNOWN:
					ret = "SOURCE_UNKNOWN";
					break;
				case NO_RESOURCES:
					ret = "NO_RESOURCES";
					break;
				case NO_RESPONSE:
					ret = "NO_RESPONSE";
					break;
				case GATEWAY_DOWN:
					ret = "GATEWAY_DOWN";
					break;
				case SYMBOL_UNKNOWN:
					ret = "SYMBOL_UNKNOWN";
					break;
				case NOT_OPEN:
					ret = "NOT_OPEN";
					break;
				case INVALID_CONTENT:
					ret = "INVALID_CONTENT";
					break;
				default:
					ret = Convert.ToString(nakCode);
					break;
			}

			return ret;
		}
	}

}