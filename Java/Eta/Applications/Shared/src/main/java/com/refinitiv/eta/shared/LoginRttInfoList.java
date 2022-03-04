/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRTT;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;

public class LoginRttInfoList implements Iterable<LoginRttInfo> {

    private List<LoginRttInfo> loginRttInfoList = new ArrayList<>(ProviderSession.NUM_CLIENT_SESSIONS);

    public LoginRttInfoList() {
        for (int i = 0; i < ProviderSession.NUM_CLIENT_SESSIONS; i++) {
            loginRttInfoList.add(new LoginRttInfo());
        }
    }

    /**
     * Initialize login rtt info watch list.
     */
    public void init() {
        for (LoginRttInfo loginRttInfo : loginRttInfoList) {
            loginRttInfo.clear();
        }
    }

    /**
     * Gets a login rtt information for a channel with same stream id.
     *
     * @param channel  - The channel to get the login request information
     *                 structure for the message
     * @param loginRTT - The LoginRTT containing the stream id
     * @return {@link LoginRttInfo} for the channel, null if login stream id
     * is different for the same channel or maximum login limit is
     * reached.
     */
    public LoginRttInfo get(Channel channel, LoginRTT loginRTT) {
        LoginRttInfo loginRttInfo = null;
        for (LoginRttInfo loginRttInfoTmp : loginRttInfoList) {
            if (loginRttInfoTmp.isInUse() && loginRttInfoTmp.channel().equals(channel)) {
                loginRttInfo = loginRttInfoTmp;
                if (loginRttInfo.loginRtt().streamId() != loginRTT.streamId()) {
                    return null;
                }
            }
        }

        // get a new one if one is not already in use
        if (loginRttInfo == null) {
            for (LoginRttInfo loginRttInfoTmp : loginRttInfoList) {
                if (!loginRttInfoTmp.isInUse()) {
                    loginRttInfo = loginRttInfoTmp;
                    loginRttInfo.channel(channel);
                    loginRttInfo.setInUse(true);
                    loginRTT.copy(loginRttInfo.loginRtt());
                    break;
                }
            }
        }

        return loginRttInfo;
    }

    /**
     * Finds a login request information for the channel.
     *
     * @param channel - The channel to get the login request information for.
     * @return {@link LoginRequestInfo} for the channel
     */
    public LoginRttInfo get(Channel channel) {
        return loginRttInfoList.stream()
                .filter(loginRttInfo -> loginRttInfo.isInUse() && loginRttInfo.channel().equals(channel))
                .findAny()
                .orElse(null);
    }

    public LoginRttInfo createFromRequest(Channel channel, LoginRequest loginRequest) {
        final LoginRttInfo appropriateLoginRttInfo = loginRttInfoList.stream()
                .filter(loginRttInfo -> !loginRttInfo.isInUse())
                .findFirst()
                .orElse(null);
        if (appropriateLoginRttInfo != null) {
            appropriateLoginRttInfo.loginRtt().initRTT(loginRequest.streamId());
            appropriateLoginRttInfo.channel(channel);
            appropriateLoginRttInfo.setInUse(true);
        }
        return appropriateLoginRttInfo;
    }

    public void clearForStream(int streamId) {
        loginRttInfoList.removeIf(loginRttInfo ->
                loginRttInfo.isInUse() && Objects.equals(streamId, loginRttInfo.loginRtt().streamId()));
    }

    @Override
    public Iterator<LoginRttInfo> iterator() {
        return loginRttInfoList.iterator();
    }
}
