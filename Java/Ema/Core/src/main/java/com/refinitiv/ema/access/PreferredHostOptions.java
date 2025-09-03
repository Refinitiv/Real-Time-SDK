///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024-2025 LSEG. All rights reserved.              --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * This class is used to modify the preferred host options via the {@link com.refinitiv.ema.access.OmmConsumer#modifyIOCtl(int, Object)} method.
 */
public class PreferredHostOptions {
    private boolean _preferredHostEnabled;
    private String _detectionTimeSchedule;
    private long _detectionTimeInterval;
    private String _channelName;
    private String _wsbChannelName;
    private boolean _fallBackWithInWSBGroup;
    private StringBuilder _stringBuilder;
    private String _sessionChannelName;

    PreferredHostOptions() {
        Clear();
    }

    /**
     * Indicates whether preferred host feature is enabled for this option.
     * @return true if preferred host is enabled; false otherwise.
     */
    public boolean isPreferredHostEnabled() {
        return _preferredHostEnabled;
    }

    /**
     * Specifies to enable or disable the preferred host feature. 
     * @param preferredHostEnabled specifies to enable this feature.
     */
    public void setPreferredHostEnabled(boolean preferredHostEnabled) {
        this._preferredHostEnabled = preferredHostEnabled;
    }

    /**
     * Returns Cron time schedule to switch over to a preferred host or WSB group.
     * @return Cron time format to switch over.
     */
    public String getDetectionTimeSchedule() {
        return _detectionTimeSchedule;
    }

    /**
     * Specifies a Cron time schedule to switch over to a preferred host or WSB group.
     * @param detectionTimeSchedule specifies a Cron time schedule.
     */
    public void setDetectionTimeSchedule(String detectionTimeSchedule) {
        this._detectionTimeSchedule = detectionTimeSchedule;
    }

    /**
     * Returns time interval in second to switch over to a preferred host or WSB group.
     * @return time interval to switch over.
     */
    public long getDetectionTimeInterval() {
        return _detectionTimeInterval;
    }

    /**
     * Specifies a time interval in second to switch over to a preferred host or WSB group.
     * @param detectionTimeInterval specifies a time interval to switch over.
     */
    public void setDetectionTimeInterval(long detectionTimeInterval) {
        this._detectionTimeInterval = detectionTimeInterval;
    }

    /**
     * Returns a channel name to set a preferred host.
     * @return the preferred host channel name.
     */
    public String getChannelName() {
        return _channelName;
    }

    /**
     * Specifies a channel name for a preferred host.
     * @param channelName specifies a preferred host.
     */
    public void setChannelName(String channelName) {
        this._channelName = channelName;
    }

    /**
     * Returns a WSB group channel name to set a preferred WSB.
     * @return the preferred WSB group channel name.
     */
    public String getWsbChannelName() {
        return _wsbChannelName;
    }

    /**
     * Specifies a WSB group channel name to set preferred WSB.
     * @param wsbChannelName specifies a preferred WSB group.
     */
    public void setWsbChannelName(String wsbChannelName) {
        this._wsbChannelName = wsbChannelName;
    }

    /**
     * Indicates whether to fallback within a WSB group instead of moving into a preferred WSB group.
     * @return true if Reactor fallbacks within a WSB group; false otherwise
     */
    public boolean isFallBackWithInWSBGroup() {
        return _fallBackWithInWSBGroup;
    }

    /**
     * Specifies whether to fallback within a WSB group instead of moving into a preferred WSB group.
     * @param fallBackWithInWSBGroup specifies to fallback within a WSB group.
     */
    public void setFallBackWithInWSBGroup(boolean fallBackWithInWSBGroup) {
        this._fallBackWithInWSBGroup = fallBackWithInWSBGroup;
    }
    
    /**
     * Returns a session channel name to apply this option for the request routing feature.
     * @return the session channel name
     */
    public String sesionChannelName() {
        return _sessionChannelName;
    }

    /**
     * Sets a session channel name to apply this option for the request routing feature.
     * @param sessionChannelName specifies the session channel name
     */
    public void sessionChannelName(String sessionChannelName) {
        this._sessionChannelName = sessionChannelName;
    }

    /**
     * Clears this object to default.
     */
    public void Clear() {
        _preferredHostEnabled = false;
        _detectionTimeSchedule = "";
        _detectionTimeInterval = 0;
        _channelName = "";
        _wsbChannelName = "";
        _fallBackWithInWSBGroup = false;
        _sessionChannelName = "";
    }

    @Override
    public String toString() {
        if (_stringBuilder == null) {
            _stringBuilder = new StringBuilder(320);
        } else {
            _stringBuilder.setLength(0);
        }

        _stringBuilder.append("\n\t\tEnablePreferredHostOptions=" + _preferredHostEnabled);
        _stringBuilder.append("\n\t\tPHDetectionTimeSchedule='" + _detectionTimeSchedule + '\'');
        _stringBuilder.append("\n\t\tPHDetectionTimeInterval=" + _detectionTimeInterval);
        _stringBuilder.append("\n\t\tPreferredChannelName='" + _channelName + '\'');
        _stringBuilder.append("\n\t\tPreferredWSBChannelName='" + _wsbChannelName + '\'');
        _stringBuilder.append("\n\t\tPHFallBackWithInWSBGroup=" + _fallBackWithInWSBGroup);
        _stringBuilder.append("\n\t\tSessionChannelName='" + _sessionChannelName + '\'');

        return _stringBuilder.toString();
    }
}
