/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.PerfTools.Common
{
	/// <summary>
	/// Generates a randomized array that can be used to determine which message in a
	/// burst should contain latency information.
	/// <para>
	/// Create the array using the <see cref="Create(LatencyRandomArrayOptions)"/> method.
	/// Iterate over the array using the <see cref="Next()"/> method.
	/// </para>
	/// </summary>
	public class LatencyRandomArray
    {
		private int m_RandArrayIndex = 0;    // Index for the randomized latency array.
		private readonly Random m_Generator = new((int)(DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond));

		/// <summary>
		/// Gets total number of values in the array.
		/// </summary>
		public int[]? Array { get; private set; }

		/// <summary>
		/// Gets the array of values.
		/// </summary>
		public int ValueCount { get; private set; }

		/// <summary>
		/// Creates a LatencyRandomArray. 
		/// </summary>
		/// <param name="opts">The options for generating latency array.</param>
		/// <returns><see cref="PerfToolsReturnCode"/></returns>
		public PerfToolsReturnCode Create(LatencyRandomArrayOptions opts)
		{
			if (opts.TotalMsgsPerSec == 0)
			{
				Console.WriteLine("Random Array: Total message rate is zero.\n");
				return PerfToolsReturnCode.FAILURE;
			}

			if (opts.LatencyMsgsPerSec == 0)
			{
				Console.WriteLine("Random Array: Latency message rate is zero.\n");
				return PerfToolsReturnCode.FAILURE;
			}

			if (opts.LatencyMsgsPerSec > opts.TotalMsgsPerSec)
			{
				Console.WriteLine("Random Array: Latency message rate is greater than total message rate.\n");
				return PerfToolsReturnCode.FAILURE;
			}

			if (opts.ArrayCount == 0)
			{
				Console.WriteLine("Random Array: Array count is zero.\n");
				return PerfToolsReturnCode.FAILURE;
			}

			int totalMsgsPerTick = opts.TotalMsgsPerSec / opts.TicksPerSec;
			int totalMsgsPerTickRemainder = opts.TotalMsgsPerSec % opts.TicksPerSec;

			ValueCount = opts.TicksPerSec * opts.ArrayCount;
			Array = new int[ValueCount];

			// Build random array.
			// The objective is to create an array that will be used for each second.
			// It will contain one value for each tick, which indicates which message
			// during the tick should contain latency information. 
			for (int setPos = 0; setPos < ValueCount; setPos += opts.TicksPerSec)
			{
				// Each array cell represents one tick.
				// Fill the array '1'with as many latency messages as we will send per second. 
				for (int i = 0; i < opts.LatencyMsgsPerSec; ++i)
					Array[setPos + i] = 1;

				// Fill the rest with -1
				for (int i = opts.LatencyMsgsPerSec; i < opts.TicksPerSec; ++i)
					Array[setPos + i] = -1;

				// Shuffle array to randomize which ticks send latency message
				for (int i = 0; i < opts.TicksPerSec; ++i)
				{
					int pos1 = Math.Abs(m_Generator.Next() % opts.TicksPerSec);
					int pos2 = Math.Abs(m_Generator.Next() % opts.TicksPerSec);
					int tmpB = Array[setPos + pos1];
					Array[setPos + pos1] = Array[setPos + pos2];
					Array[setPos + pos2] = tmpB;
				}

				// Now, for each tick that sends a latency message, determine which message that will be 
				for (int i = 0; i < opts.TicksPerSec; ++i)
					if (Array[setPos + i] == 1)
						Array[setPos + i] = Math.Abs(m_Generator.Next() % totalMsgsPerTick + ((i < totalMsgsPerTickRemainder) ? 1 : 0));
			}

			return PerfToolsReturnCode.SUCCESS;
		}

		/// <summary>
		/// Iterates over the LatencyRandomArray. The value returned indicates which
		/// message in the tick should contain latency information. If the value is
		/// -1, no latency message should be sent in that tick. The iteration starts
		/// over when the end of the array is reached.
		/// </summary>
		/// <returns>value indicating which message in the tick contains latency information</returns>
		public int Next()
		{
			m_RandArrayIndex++;
			if (m_RandArrayIndex == ValueCount)
			{
				m_RandArrayIndex = 0;
			}
			return Array![m_RandArrayIndex];
		}
	}
}
