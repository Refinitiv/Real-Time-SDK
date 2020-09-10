package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.valueadd.domainrep.rdm.login.LoginMsg;

/**
 * (Consumers only) Provides information about a received authentication response.
 */
public class TunnelStreamAuthInfo
{
    LoginMsg _loginMsg;
    
    /**
     *  The received authentication message. Null if TunnelStream authentication is not enabled.
     *
     * @return the login msg
     */
    public LoginMsg loginMsg()
    {
        return _loginMsg;
    }
    
    /**
     * Login msg.
     *
     * @param loginMsg the login msg
     */
    void loginMsg(LoginMsg loginMsg)
    {
        _loginMsg = loginMsg;
    }

    /**
     * Clear.
     */
    void clear()
    {
        _loginMsg = null; 
    }
}
