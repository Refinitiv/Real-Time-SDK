///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

public class PreferredHostOptions {
    private boolean _preferredHostEnabled;
    private String _detectionTimeSchedule;
    private long _detectionTimeInterval;
    private String _channelName;
    private String _wsbChannelName;
    private boolean _fallBackWithInWSBGroup;
    private StringBuilder _stringBuilder;

    PreferredHostOptions() {
        Clear();
    }

    /**
     * Indicates whether preferred host feature is configured for this channel.
     *
     * @return true if preferred host is enabled; false otherwise
     */
    public boolean isPreferredHostEnabled() {
        return _preferredHostEnabled;
    }

    public void setPreferredHostEnabled(boolean preferredHostEnabled) {
        this._preferredHostEnabled = preferredHostEnabled;
    }

    /**
     * Returns Cron time schedule to switch over to a preferred host or WSB group.
     *
     * @return time format to switch over
     */
    public String getDetectionTimeSchedule() {
        return _detectionTimeSchedule;
    }

    public void setDetectionTimeSchedule(String detectionTimeSchedule) {
        this._detectionTimeSchedule = detectionTimeSchedule;
    }

    /**
     * Returns time interval in second to switch over to a preferred host or WSB group.
     *
     * @return time interval to switch over
     */
    public long getDetectionTimeInterval() {
        return _detectionTimeInterval;
    }

    public void setDetectionTimeInterval(long detectionTimeInterval) {
        this._detectionTimeInterval = detectionTimeInterval;
    }

    /**
     * Returns a channel name to set a preferred host.
     *
     * @return the preferred host channel name
     */
    public String getChannelName() {
        return _channelName;
    }

    public void setChannelName(String channelName) {
        this._channelName = channelName;
    }

    /**
     * Returns a WSB group channel name to set a preferred WSB.
     *
     * @return the preferred WSB group channel name
     */
    public String getWsbChannelName() {
        return _wsbChannelName;
    }

    public void setWsbChannelName(String wsbChannelName) {
        this._wsbChannelName = wsbChannelName;
    }

    /**
     * Indicates whether to fallback within a WSB group instead of moving into a preferred WSB group.
     *
     * @return true if Reactor fallbacks within a WSB group; false otherwise
     */
    public boolean isFallBackWithInWSBGroup() {
        return _fallBackWithInWSBGroup;
    }

    public void setFallBackWithInWSBGroup(boolean fallBackWithInWSBGroup) {
        this._fallBackWithInWSBGroup = fallBackWithInWSBGroup;
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
    }

    @Override
    public String toString() {
        if (_stringBuilder == null) {
            _stringBuilder = new StringBuilder(256);
        } else {
            _stringBuilder.setLength(0);
        }

        _stringBuilder.append("\n\t\tEnablePreferredHostOptions=" + _preferredHostEnabled);
        _stringBuilder.append("\n\t\tPHDetectionTimeSchedule='" + _detectionTimeSchedule + '\'');
        _stringBuilder.append("\n\t\tPHDetectionTimeInterval=" + _detectionTimeInterval);
        _stringBuilder.append("\n\t\tPreferredChannelName='" + _channelName + '\'');
        _stringBuilder.append("\n\t\tPreferredWSBChannelName='" + _wsbChannelName + '\'');
        _stringBuilder.append("\n\t\tPHFallBackWithInWSBGroup=" + _fallBackWithInWSBGroup);

        return _stringBuilder.toString();
    }
}
