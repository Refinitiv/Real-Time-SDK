/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;

/**
 * Login request information list.
 */
public class LoginRequestInfoList implements Iterable<LoginRequestInfo>
{
    private List<LoginRequestInfo> _loginRequestInfoList = new ArrayList<LoginRequestInfo>(ProviderSession.NUM_CLIENT_SESSIONS);

    public LoginRequestInfoList()
    {
        for (int i = 0; i < ProviderSession.NUM_CLIENT_SESSIONS; i++)
        {
            _loginRequestInfoList.add(new LoginRequestInfo());
        }
    }

    /**
     * Initialize login request info watch list.
     */
    public void init()
    {
        for (LoginRequestInfo loginRequestInfo : _loginRequestInfoList)
        {
            loginRequestInfo.clear();
        }
    }

    /**
     * Gets a login request information for a channel with same stream id.
     * 
     * @param channel - The channel to get the login request information
     *            structure for the message
     * @param loginRequest - The loginRequest containing the stream id
     * @return {@link LoginRequestInfo} for the channel, null if login stream id
     *         is different for the same channel or maximum login limit is
     *         reached.
     */
    public LoginRequestInfo get(Channel channel, LoginRequest loginRequest)
    {
        LoginRequestInfo loginRequestInfo = null;
        for (LoginRequestInfo loginRequestInfoTmp : _loginRequestInfoList)
        {
            if (loginRequestInfoTmp.isInUse && loginRequestInfoTmp.channel == channel)
            {
                loginRequestInfo = loginRequestInfoTmp;
                if (loginRequestInfo.loginRequest.streamId() != loginRequest.streamId())
                    return null;
            }
        }

        // get a new one if one is not already in use
        if (loginRequestInfo == null)
        {
            for (LoginRequestInfo loginRequestInfoTmp : _loginRequestInfoList)
            {
                if (!loginRequestInfoTmp.isInUse)
                {
                    loginRequestInfo = loginRequestInfoTmp;
                    loginRequestInfo.channel = channel;
                    loginRequestInfo.isInUse = true;
                    loginRequest.copy(loginRequestInfo.loginRequest);
                    break;
                }
            }
        }

        return loginRequestInfo;
    }

    /**
     * Finds a login request information for the channel.
     * 
     * @param chnl - The channel to get the login request information for.
     * 
     * @return {@link LoginRequestInfo} for the channel
     */
    public LoginRequestInfo get(Channel chnl)
    {
        for (LoginRequestInfo loginRequestInfo : _loginRequestInfoList)
        {
            if (loginRequestInfo.isInUse && loginRequestInfo.channel == chnl)
            {
                return loginRequestInfo;
            }
        }

        return null;
    }

    @Override
    public Iterator<LoginRequestInfo> iterator()
    {
        return _loginRequestInfoList.iterator();
    }
}
