/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;

namespace LSEG.Eta.PerfTools.Common
{
    public class ResourceUsageStats
    {
        private double m_PreviousTimeStamp;
        private double m_PreviousCpuUsageTime;
        private Process m_Process;
        private double m_TotalCpuUsage;

        public ResourceUsageStats()
        {
            m_Process = Process.GetCurrentProcess();
        }

        /// <summary>
        /// Initializes resource usage statistics.
        /// </summary>
        public void Init()
        {
            m_PreviousTimeStamp = GetTime.GetMilliseconds();
            m_PreviousCpuUsageTime = m_Process.TotalProcessorTime.TotalMilliseconds;
        }

        /// <summary>
        /// Refreshes resource usage.
        /// </summary>
        /// <remarks>
        /// Calls this function to get usage statistics
        /// </remarks>
        public void Refresh()
        {
            double currentTimeStamp = GetTime.GetMilliseconds();
            double cpuUsageTime = m_Process.TotalProcessorTime.TotalMilliseconds;
            double cpuUsage = cpuUsageTime - m_PreviousCpuUsageTime;
            double timePassed = currentTimeStamp - m_PreviousTimeStamp;
            m_TotalCpuUsage = cpuUsage / (Environment.ProcessorCount * timePassed);

            /* Keeps previous usage for next refresh*/
            m_PreviousTimeStamp = currentTimeStamp;
            m_PreviousCpuUsageTime = cpuUsageTime;
        }

        /// <summary>
        /// Gets the amount of CPU usage time.
        /// </summary>
        /// <returns></returns>
        public double CurrentProcessCpuLoad()
        {
            return m_TotalCpuUsage * 100;
        }

        /// <summary>
        /// Gets the amount of memory usage in bytes
        /// </summary>
        /// <returns></returns>
        public double CurrentMemoryUsage()
        {
            return m_Process.PeakWorkingSet64/(1024 * 1024);
        }
    }
}
