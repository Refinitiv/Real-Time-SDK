/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRTT;

public class LoginRttInfo {

    private LoginRTT loginRtt;
    private Channel channel;
    private boolean inUse;
    private long rttLastSendNanoTime;

    public LoginRttInfo() {
        loginRtt = (LoginRTT) LoginMsgFactory.createMsg();
        loginRtt.rdmMsgType(LoginMsgType.RTT);
        rttLastSendNanoTime(System.nanoTime());
    }

    public void clear() {
        loginRtt.clear();
        channel = null;
        inUse = false;
    }

    public LoginRTT loginRtt() {
        return loginRtt;
    }

    public Channel channel() {
        return channel;
    }

    public boolean isInUse() {
        return inUse;
    }

    public void setInUse(boolean inUse) {
        this.inUse = inUse;
    }

    public void channel(Channel channel) {
        this.channel = channel;
    }

    public void rttLastSendNanoTime(long rttLastSendNanoTime) {
        this.rttLastSendNanoTime = rttLastSendNanoTime;
    }

    public long rttLastSendNanoTime() {
        return this.rttLastSendNanoTime;
    }
}
