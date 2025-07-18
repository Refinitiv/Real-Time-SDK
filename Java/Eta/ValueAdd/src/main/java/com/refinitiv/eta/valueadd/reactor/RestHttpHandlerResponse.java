/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;



class RestHttpHandlerResponse {

    enum Action {
        NO_ACTION,
        SEND_BASIC_AUTH_REQUEST,
        SEND_KERBOROS_AUTH,
        SEND_NTLM_AUTH
    }

    int returnCode;
    boolean finished;
    Action action;

    RestHttpHandlerResponse() {
        this.returnCode = ReactorReturnCodes.SUCCESS;
        this.finished = true;
        this.action = Action.NO_ACTION;
    }

    RestHttpHandlerResponse(int returnCode) {
        this.returnCode = returnCode;
        this.finished = true;
        this.action = Action.NO_ACTION;
    }

    RestHttpHandlerResponse(int returnCode, boolean finished, Action action) {
        this.returnCode = returnCode;
        this.finished = finished;
        this.action = action;
    }
}
