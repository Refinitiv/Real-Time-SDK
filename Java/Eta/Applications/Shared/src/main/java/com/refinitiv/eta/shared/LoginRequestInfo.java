/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;

/** Login request information. */
public class LoginRequestInfo
{
    public LoginRequest loginRequest;
    Channel channel;
    boolean isInUse;

    public LoginRequestInfo()
    {
        loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
        loginRequest.rdmMsgType(LoginMsgType.REQUEST);
    }

    public void clear()
    {
        loginRequest.clear();
        channel = null;
        isInUse = false;
    }
    
    public LoginRequest loginRequest()
    {
        return loginRequest;
    }
    
    public Channel channel()
    {
        return channel;
    }
    
    public boolean isInUse()
    {
        return isInUse;
    }
}
