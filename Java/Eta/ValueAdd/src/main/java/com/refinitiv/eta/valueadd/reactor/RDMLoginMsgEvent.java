package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;

/**
 * Event provided to RDMLoginMsgCallback methods.
 * 
 * @see ReactorMsgEvent
 */
public class RDMLoginMsgEvent extends ReactorMsgEvent
{
    LoginMsg _loginMsg;

    RDMLoginMsgEvent()
    {
        super();
    }

    void rdmLoginMsg(LoginMsg rdmLoginMsg)
    {
        _loginMsg = rdmLoginMsg;
    }

    /**
     * The LoginMsg associated with this message event.
     * 
     * @return LoginMsg
     */
    public LoginMsg rdmLoginMsg()
    {
        return _loginMsg;
    }
}
