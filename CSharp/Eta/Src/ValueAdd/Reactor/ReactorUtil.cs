/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    internal static class ReactorUtil
    {
	static ReactorUtil()
	{
		TicksPerSecond = Stopwatch.Frequency;
            	TicksPerMilliSecond = Stopwatch.Frequency / 1000.0;
	}	

	public static double TicksPerSecond { get; private set; }

	public static double TicksPerMilliSecond { get; private set; }

        public static long GetCurrentTimeMilliSecond()
        {
            return (long)(Stopwatch.GetTimestamp() / TicksPerMilliSecond);
        }

        public static long GetCurrentTimeSecond()
        {
            return (long)(Stopwatch.GetTimestamp() / TicksPerSecond);
        }
    }
}
