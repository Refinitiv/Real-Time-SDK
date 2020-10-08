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
