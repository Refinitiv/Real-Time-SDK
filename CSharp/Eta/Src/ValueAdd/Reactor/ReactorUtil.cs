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
    internal class ReactorUtil
    {
        public static long GetCurrentTimeMilliSecond()
        {
            return (Stopwatch.GetTimestamp() / TimeSpan.TicksPerMillisecond);
        }

        public static long GetCurrentTimeSecond()
        {
            return (Stopwatch.GetTimestamp() / TimeSpan.TicksPerSecond);
        }
    }
}
