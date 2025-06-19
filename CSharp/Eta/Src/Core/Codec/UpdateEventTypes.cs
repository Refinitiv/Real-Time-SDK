/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Rdm
{
    /// <summary>
    /// Indicates the general content of the update.
    /// </summary>
    sealed public class UpdateEventTypes
	{
		// UpdateEventTypes class cannot be instantiated
		private UpdateEventTypes()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// Unspecified Update Event </summary>
		public const int UNSPECIFIED = 0;
		/// <summary>
		/// Update Event Quote </summary>
		public const int QUOTE = 1;
		/// <summary>
		/// Update Event Trade </summary>
		public const int TRADE = 2;
		/// <summary>
		/// Update Event News Alert </summary>
		public const int NEWS_ALERT = 3;
		/// <summary>
		/// Update Event Volume Alert </summary>
		public const int VOLUME_ALERT = 4;
		/// <summary>
		/// Update Event Order Indication </summary>
		public const int ORDER_INDICATION = 5;
		/// <summary>
		/// Update Event Closing Run </summary>
		public const int CLOSING_RUN = 6;
		/// <summary>
		/// Update Event Correction </summary>
		public const int CORRECTION = 7;
		/// <summary>
		/// Update Event Market Digest </summary>
		public const int MARKET_DIGEST = 8;
		/// <summary>
		/// Update Event Quotes followed by a Trade </summary>
		public const int QUOTES_TRADE = 9;
		/// <summary>
		/// Update Event with filtering and conflation applied </summary>
		public const int MULTIPLE = 10;
		/// <summary>
		/// Fields may have changed </summary>
		public const int VERIFY = 11;
		/* Maximum reserved update event type */
		internal const int MAX_RESERVED = 127;

		/// <summary>
		/// String representation of an update event type name.
		/// </summary>
		/// <param name="updateEventType"> update event type value
		/// </param>
		/// <returns> the string representation of an update event type name
		/// </returns>
		public static string ToString(int updateEventType)
		{
			string ret = "";

			switch (updateEventType)
			{
				case UNSPECIFIED:
					ret = "UNSPECIFIED";
					break;
				case QUOTE:
					ret = "QUOTE";
					break;
				case TRADE:
					ret = "TRADE";
					break;
				case NEWS_ALERT:
					ret = "NEWS_ALERT";
					break;
				case VOLUME_ALERT:
					ret = "VOLUME_ALERT";
					break;
				case ORDER_INDICATION:
					ret = "ORDER_INDICATION";
					break;
				case CLOSING_RUN:
					ret = "CLOSING_RUN";
					break;
				case CORRECTION:
					ret = "CORRECTION";
					break;
				case MARKET_DIGEST:
					ret = "MARKET_DIGEST";
					break;
				case QUOTES_TRADE:
					ret = "QUOTES_TRADE";
					break;
				case MULTIPLE:
					ret = "MULTIPLE";
					break;
				case VERIFY:
					ret = "VERIFY";
					break;
				default:
					ret = System.Convert.ToString(updateEventType);
					break;
			}

			return ret;
		}

	}

}