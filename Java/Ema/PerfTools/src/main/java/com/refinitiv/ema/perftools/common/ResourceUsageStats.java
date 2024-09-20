/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.lang.management.ManagementFactory;
import com.sun.management.OperatingSystemMXBean;

/**
 *  Resource Statistics (CPU and Memory Usage).
 */
@SuppressWarnings("restriction")
public class ResourceUsageStats
{
    // note: Java 7 (Oracle JDK) introduced the following method for obtaining the current proceses's CPU usage.
    // See http://sellmic.com/blog/2011/07/21/hidden-java-7-features-cpu-load-monitoring/ for details
    private static OperatingSystemMXBean osBean = ManagementFactory.getPlatformMXBean(OperatingSystemMXBean.class);
    
    /**
     *  CPU load of the process.
     *
     * @return the double
     */
    public static double currentProcessCpuLoad()
    {
    	// What % CPU load this current JVM is taking, from 0.0-1.0
    	double cpuLoad = osBean.getProcessCpuLoad() * 100 * osBean.getAvailableProcessors();
    	
    	return (cpuLoad >= 0 ? cpuLoad : 0);
    }
    
    /**
     *  Memory usage of the process.
     *
     * @return the double
     */
    public static double currentMemoryUsage()
    {
    	double memorySize = osBean.getCommittedVirtualMemorySize() / 1048576.0;
    	
    	return (memorySize >= 0 ? memorySize : 0);
    }
}
