/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;

public class QosWrapper {

    private int rate;
    private int rateInfo;
    private int timeliness;
    private int timeInfo;
    private boolean dynamic;

    public void rate(String value) {
        this.rate = qosRateValue(value);
    }

    public void rateInfo(String value) {
        this.rateInfo = Integer.parseInt(value);
    }

    public void timeliness(String value) {
        this.timeliness = qosTimelinessValue(value);
    }

    public void timeInfo(String value) {
        this.timeInfo = Integer.parseInt(value);
    }

    public void dynamic(String value) {
        this.dynamic = Integer.parseInt(value) > 0;
    }

    public int rate() {
        return rate;
    }

    public int rateInfo() {
        return rateInfo;
    }

    public int timeliness() {
        return timeliness;
    }

    public int timeInfo() {
        return timeInfo;
    }

    public boolean dynamic() {
        return dynamic;
    }

    public void clear() {
        this.rate = QosTimeliness.UNSPECIFIED;
        this.rateInfo = 0;
        this.timeliness = QosTimeliness.UNSPECIFIED;
        this.timeInfo = 0;
        this.dynamic = false;
    }

    /* Converts qos timeliness string to qos timeliness value. */
    private int qosTimelinessValue(String value) {
        int retVal = 0;

        if (value.equals("RSSL_QOS_TIME_UNSPECIFIED")) {
            retVal = QosTimeliness.UNSPECIFIED;
        } else if (value.equals("RSSL_QOS_TIME_REALTIME")) {
            retVal = QosTimeliness.REALTIME;
        } else if (value.equals("RSSL_QOS_TIME_DELAYED_UNKNOWN")) {
            retVal = QosTimeliness.DELAYED_UNKNOWN;
        } else if (value.equals("RSSL_QOS_TIME_DELAYED")) {
            retVal = QosTimeliness.DELAYED;
        }

        return retVal;
    }

    /* Converts qos rate string to qos rate value. */
    private int qosRateValue(String value) {
        int retVal = 0;

        if (value.equals("RSSL_QOS_RATE_UNSPECIFIED")) {
            retVal = QosRates.UNSPECIFIED;
        } else if (value.equals("RSSL_QOS_RATE_TICK_BY_TICK")) {
            retVal = QosRates.TICK_BY_TICK;
        } else if (value.equals("RSSL_QOS_RATE_JIT_CONFLATED")) {
            retVal = QosRates.JIT_CONFLATED;
        } else if (value.equals("RSSL_QOS_RATE_TIME_CONFLATED")) {
            retVal = QosRates.TIME_CONFLATED;
        }

        return retVal;
    }
}
