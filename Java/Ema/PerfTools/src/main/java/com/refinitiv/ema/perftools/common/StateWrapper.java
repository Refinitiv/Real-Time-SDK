/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import com.refinitiv.ema.access.OmmState;

public class StateWrapper {
    private int streamState;
    private int dataState;
    private int statusCode;
    private String statusText;

    public void streamState(String value) {
        this.streamState = streamStateValue(value);
    }

    public int streamState() {
        return streamState;
    }

    public void dataState(String value) {
        this.dataState = dataStateValue(value);
    }

    public int dataState() {
        return dataState;
    }

    public void statusCode(String statusCode) {
        this.statusCode = codeValue(statusCode);
    }

    public int statusCode() {
        return statusCode;
    }

    public void statusText(String statusText) {
        this.statusText = statusText;
    }

    public String statusText() {
        return statusText;
    }

    public void clear() {
        streamState = 0;
        dataState = OmmState.DataState.SUSPECT;
        statusCode = OmmState.StatusCode.NONE;
        statusText = "";
    }

    /* Converts stream state string to stream state value. */
    private int streamStateValue(String value) {
        int retVal = 0;

        if (value.equals("RSSL_STREAM_OPEN")) {
            retVal = OmmState.StreamState.OPEN;
        } else if (value.equals("RSSL_STREAM_NON_STREAMING")) {
            retVal = OmmState.StreamState.NON_STREAMING;
        } else if (value.equals("RSSL_STREAM_CLOSED_RECOVER")) {
            retVal = OmmState.StreamState.CLOSED_RECOVER;
        } else if (value.equals("RSSL_STREAM_CLOSED")) {
            retVal = OmmState.StreamState.CLOSED;
        } else if (value.equals("RSSL_STREAM_REDIRECTED")) {
            retVal = OmmState.StreamState.CLOSED_REDIRECTED;
        }

        return retVal;
    }

    /* Converts data state string to data state value. */
    private int dataStateValue(String value) {
        int retVal = 0;

        if (value.equals("RSSL_DATA_NO_CHANGE")) {
            retVal = OmmState.DataState.NO_CHANGE;
        } else if (value.equals("RSSL_DATA_OK")) {
            retVal = OmmState.DataState.OK;
        } else if (value.equals("RSSL_DATA_SUSPECT")) {
            retVal = OmmState.DataState.SUSPECT;
        }

        return retVal;
    }

    /* Converts state code string to state code value. */
    private int codeValue(String value) {
        int retVal = OmmState.StatusCode.NONE;

        if (value.equals("RSSL_SC_NOT_FOUND")) {
            retVal = OmmState.StatusCode.NOT_FOUND;
        } else if (value.equals("RSSL_SC_TIMEOUT")) {
            retVal = OmmState.StatusCode.TIMEOUT;
        } else if (value.equals("RSSL_SC_NOT_ENTITLED")) {
            retVal = OmmState.StatusCode.NOT_AUTHORIZED;
        } else if (value.equals("RSSL_SC_INVALID_ARGUMENT")) {
            retVal = OmmState.StatusCode.INVALID_ARGUMENT;
        } else if (value.equals("RSSL_SC_USAGE_ERROR")) {
            retVal = OmmState.StatusCode.USAGE_ERROR;
        } else if (value.equals("RSSL_SC_PREEMPTED")) {
            retVal = OmmState.StatusCode.PREEMPTED;
        } else if (value.equals("RSSL_SC_JIT_CONFLATION_STARTED")) {
            retVal = OmmState.StatusCode.JUST_IN_TIME_CONFLATION_STARTED;
        } else if (value.equals("RSSL_SC_REALTIME_RESUMED")) {
            retVal = OmmState.StatusCode.TICK_BY_TICK_RESUMED;
        } else if (value.equals("RSSL_SC_FAILOVER_STARTED")) {
            retVal = OmmState.StatusCode.FAILOVER_STARTED;
        } else if (value.equals("RSSL_SC_FAILOVER_COMPLETED")) {
            retVal = OmmState.StatusCode.FAILOVER_COMPLETED;
        } else if (value.equals("RSSL_SC_GAP_DETECTED")) {
            retVal = OmmState.StatusCode.GAP_DETECTED;
        } else if (value.equals("RSSL_SC_NO_RESOURCES")) {
            retVal = OmmState.StatusCode.NO_RESOURCES;
        } else if (value.equals("RSSL_SC_TOO_MANY_ITEMS")) {
            retVal = OmmState.StatusCode.TOO_MANY_ITEMS;
        } else if (value.equals("RSSL_SC_ALREADY_OPEN")) {
            retVal = OmmState.StatusCode.ALREADY_OPEN;
        } else if (value.equals("RSSL_SC_SOURCE_UNKNOWN")) {
            retVal = OmmState.StatusCode.SOURCE_UNKNOWN;
        } else if (value.equals("RSSL_SC_NOT_OPEN")) {
            retVal = OmmState.StatusCode.NOT_OPEN;
        } else if (value.equals("RSSL_SC_NON_UPDATING_ITEM")) {
            retVal = OmmState.StatusCode.NON_UPDATING_ITEM;
        } else if (value.equals("RSSL_SC_UNSUPPORTED_VIEW_TYPE")) {
            retVal = OmmState.StatusCode.UNSUPPORTED_VIEW_TYPE;
        } else if (value.equals("RSSL_SC_INVALID_VIEW")) {
            retVal = OmmState.StatusCode.INVALID_VIEW;
        } else if (value.equals("RSSL_SC_FULL_VIEW_PROVIDED")) {
            retVal = OmmState.StatusCode.FULL_VIEW_PROVIDED;
        } else if (value.equals("RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH")) {
            retVal = OmmState.StatusCode.UNABLE_TO_REQUEST_AS_BATCH;
        } else if (value.equals("RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ")) {
            retVal = OmmState.StatusCode.NO_BATCH_VIEW_SUPPORT_IN_REQ;
        } else if (value.equals("RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER")) {
            retVal = OmmState.StatusCode.EXCEEDED_MAX_MOUNTS_PER_USER;
        } else if (value.equals("RSSL_SC_ERROR")) {
            retVal = OmmState.StatusCode.ERROR;
        } else if (value.equals("RSSL_SC_DACS_DOWN")) {
            retVal = OmmState.StatusCode.DACS_DOWN;
        } else if (value.equals("RSSL_SC_USER_UNKNOWN_TO_PERM_SYS")) {
            retVal = OmmState.StatusCode.USER_UNKNOWN_TO_PERM_SYS;
        } else if (value.equals("RSSL_SC_DACS_MAX_LOGINS_REACHED")) {
            retVal = OmmState.StatusCode.DACS_MAX_LOGINS_REACHED;
        } else if (value.equals("RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED")) {
            retVal = OmmState.StatusCode.DACS_USER_ACCESS_TO_APP_DENIED;
        } else if (value.equals("GAP_FILL")) {
            retVal = OmmState.StatusCode.GAP_FILL;
        } else if (value.equals("APP_AUTHORIZATION_FAILED")) {
            retVal = OmmState.StatusCode.APP_AUTHORIZATION_FAILED;
        }
        return retVal;
    }
}
