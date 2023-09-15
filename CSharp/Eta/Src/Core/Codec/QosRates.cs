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
	/// Quality of service rates enumerations convey information about the data's period of change.
	/// </summary>
	sealed public class QosRates
	{
		// QosRates class cannot be instantiated
		private QosRates()
		{
		}

		/// <summary>
		/// Qos Unspecified, indicates initialized structure and not intended to be encoded
		/// </summary>
		public const int UNSPECIFIED = 0;

		/// <summary>
		/// Qos Tick By Tick, indicates every change to information is conveyed </summary>
		public const int TICK_BY_TICK = 1;

		/// <summary>
		/// Just In Time Conflation, indicates extreme bursts of data may be conflated
		/// </summary>
		public const int JIT_CONFLATED = 2;

		/// <summary>
		/// Time Conflated (in ms), where conflation time is provided in rateInfo </summary>
		public const int TIME_CONFLATED = 3;

		/// <summary>
		/// Provide string representation for a Qos quality rate value.
		/// </summary>
		/// <param name="rate">rate value to represent</param>
		/// <returns> string representation for a Qos quality rate value
		/// </returns>
		/// <seealso cref="Qos"/>
		public static string ToString(int rate)
		{
			switch (rate)
			{
				case QosRates.UNSPECIFIED:
					return "Unspecified";
				case QosRates.TICK_BY_TICK:
					return "TickByTick";
				case QosRates.JIT_CONFLATED:
					return "JustInTimeConflated";
				case QosRates.TIME_CONFLATED:
					return "ConflatedByRateInfo";
				default:
					return Convert.ToString(rate);
			}
		}
	}

}