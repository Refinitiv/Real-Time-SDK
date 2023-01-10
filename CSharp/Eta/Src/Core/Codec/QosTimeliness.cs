/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// Quality Timeliness enumerations describe the age of the data.
	/// </summary>
	public class QosTimeliness
	{
		// QosTimeliness class cannot be instantiated
		private QosTimeliness()
		{
		}

		/// <summary>
		/// timeliness is unspecified. Typically used by QoS initialization methods
		/// and not intended to be encoded or decoded.
		/// </summary>
		public const int UNSPECIFIED = 0;

		/// <summary>
		/// timeliness is Real Time: data is updated as soon as new data becomes
		/// available. This is the highest-quality timeliness value. Real Time in
		/// conjunction with a rate of <see cref="QosRates.TICK_BY_TICK"/> is the best overall QoS.
		/// </summary>
		public const int REALTIME = 1;

		/// <summary>
		/// timeliness is delayed, though the amount of delay is unknown. This is a
		/// lower quality than <see cref="REALTIME"/> and might be worse than
		/// <see cref="DELAYED"/> (in which case the delay is known).
		/// </summary>
		public const int DELAYED_UNKNOWN = 2;

		/// <summary>
		/// timeliness is delayed and the amount of delay is provided in
		/// <see cref="Qos.TimeInfo()"/>. This is lower quality than <see cref="REALTIME"/> and
		/// might be better than <see cref="DELAYED_UNKNOWN"/>.
		/// </summary>
		public const int DELAYED = 3;

		/// <summary>
		/// Provides string representation for a Qos quality timeliness value.
		/// </summary>
		/// <param name="timeliness">Qos quality timeliness value</param>
		/// <returns> string representation for a Qos quality timeliness value
		/// </returns>
		/// <seealso cref="Qos"/>
		public static string ToString(int timeliness)
		{
			switch (timeliness)
			{
				case QosTimeliness.UNSPECIFIED:
					return "Unspecified";
				case QosTimeliness.REALTIME:
					return "Realtime";
				case QosTimeliness.DELAYED_UNKNOWN:
					return "DelayedByUnknown";
				case QosTimeliness.DELAYED:
					return "DelayedByTimeInfo";
				default:
					return Convert.ToString(timeliness);
			}
		}
	}

}