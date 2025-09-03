/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2025 LSEG. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Configuration options for specifying a preferred host to switch over when the
 * connection is lost or at specified detection times or intervals.
 */

public class ReactorPreferredHostOptions {
	
    private boolean _isPreferredHostEnabled;
    private String _detectionTimeSchedule;
    private long _detectionTimeInterval;
    private int _connectionListIndex;
    private int _warmStandbyGroupListIndex;
    private boolean _fallBackWithInWSBGroup;
    
     
    ReactorPreferredHostOptions()
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
     * Sets whether preferred host feature is configured for this channel.
     *
     * @param isPreferredHostEnabled the preferred host is enabled
     */
    public void isPreferredHostEnabled(boolean isPreferredHostEnabled)
    {
    	_isPreferredHostEnabled = isPreferredHostEnabled;
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
     * Sets Cron time schedule to switch over to a preferred host or WSB group.
     *
     * @param detectionTimeSchedule the string cron expression
     */
    public void detectionTimeSchedule(String detectionTimeSchedule)
    {
    	_detectionTimeSchedule = detectionTimeSchedule;
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
     * Sets time interval in second to switch over to a preferred host or WSB
     * group.
     *
     * @param detectionTimeInterval the detection time interval
     */
    public void detectionTimeInterval(long detectionTimeInterval) {
        _detectionTimeInterval = detectionTimeInterval;
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
     * Sets the index of {@link ReactorConnectOptions#connectionList()} to set a
     * preferred host.
     *
     * @param connectionListIndex the preferred host index
     */
    public void connectionListIndex(int connectionListIndex) {
        _connectionListIndex = connectionListIndex;
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
     * Sets an index of
     * {@link ReactorConnectOptions#reactorWarmStandbyGroupList()} to set a
     * preferred WSB.
     *
     * @param warmStandbyGroupListIndex the preferred WSB group index
     */
    public void warmStandbyGroupListIndex(int warmStandbyGroupListIndex) {
        _warmStandbyGroupListIndex = warmStandbyGroupListIndex;
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
     * Sets boolean value of whether to fallback within a warmstandby group. 
     * If true, the reactorChannel will fallback within a warmstandby group to the starting active channel when possible.
     *
     * @param fallBackWithInWSBGroup fallback within warmstandby group
     */
    public void fallBackWithInWSBGroup(boolean fallBackWithInWSBGroup) {
        _fallBackWithInWSBGroup = fallBackWithInWSBGroup;
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
        _fallBackWithInWSBGroup = false;
    }
    
    /**
     * Performs deep copy to the passed in parameter
     * 
     * @param destOptions the parameter getting populated with the values of the calling Object
     */
    public void copy(ReactorPreferredHostOptions destOptions)
    {
    	destOptions._isPreferredHostEnabled = _isPreferredHostEnabled;
    	destOptions._detectionTimeSchedule = _detectionTimeSchedule;
    	destOptions._detectionTimeInterval = _detectionTimeInterval;
    	destOptions._connectionListIndex = _connectionListIndex;
    	destOptions._warmStandbyGroupListIndex = _warmStandbyGroupListIndex;
    	destOptions._fallBackWithInWSBGroup = _fallBackWithInWSBGroup;
    }
}
