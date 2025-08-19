/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2025 LSEG. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * This class represents preferred host information returned by the {@link ReactorChannel#info(ReactorChannelInfo, ReactorErrorInfo)} call.
 */
public class ReactorPreferredHostInfo {
	
	boolean _isPreferredHostEnabled;
    String _detectionTimeSchedule;
    long _detectionTimeInterval;
    int _connectionListIndex;
    int _warmStandbyGroupListIndex;
    boolean _fallBackWithInWSBGroup;
    long _remainingDetectionTime;

    ReactorPreferredHostInfo()
    {
        clear();
    }
    
    /**
     * Indicates whether preferred host feature is configured for this channel.
     *
     * @return true if preferred host is enabled; false otherwise
     */
    public boolean isPreferredHostEnabled() {
        return _isPreferredHostEnabled;
    }
    
    /**
     * Returns Cron time schedule to switch over to a preferred host or WSB group.
     *
     * @return time format to switch over
     */
    public String detectionTimeSchedule() {
        return _detectionTimeSchedule;
    }
     
    /**
     * Returns time interval in second to switch over to a preferred host or WSB
     * group.
     *
     * @return time interval to switch over
     */
    public long detectionTimeInterval() {
        return _detectionTimeInterval;
    }
    
    /**
     * Returns an index of {@link ReactorConnectOptions#connectionList()} to set a
     * preferred host.
     *
     * @return the preferred host index
     */
    public int connectionListIndex() {
        return _connectionListIndex;
    }
     
    /**
     * Returns an index of
     * {@link ReactorConnectOptions#reactorWarmStandbyGroupList()} to set a
     * preferred WSB.
     *
     * @return the preferred WSB group index
     */
    public int warmStandbyGroupListIndex() {
        return _warmStandbyGroupListIndex;
    }
    
    /**
     * Gets boolean value of whether to fallback within a warmstandby group. 
     * If true, the reactorChannel will fallback within a warmstandby group to the starting active channel when possible.
     *
     * @return boolean to fallback within warmstandby group
     */
    public boolean fallBackWithInWSBGroup() {
        return _fallBackWithInWSBGroup;
    }
    
    /**
     * Gets the remaining detection time in seconds to perform fallback to preferred host.
     *
     * @return long remaining detection time
     */
    public long remainingDetectionTime() {
        return _remainingDetectionTime ;
    }
    
    /**
     * Clears this object to default.
     */
    public void clear() {
        _isPreferredHostEnabled = false;
        _detectionTimeSchedule = "";
        _detectionTimeInterval = 0;
        _connectionListIndex = 0;
        _warmStandbyGroupListIndex = 0;
        _remainingDetectionTime = 0;
        _fallBackWithInWSBGroup = false;
    }
}
