package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsg;

/**
 * (Consumers only) Provides information about a received authentication response.
 */
public class TunnelStreamAuthInfo
{
    LoginMsg _loginMsg;
    
    /** The received authentication message. Null if TunnelStream authentication is not enabled.*/
    public LoginMsg loginMsg()
    {
        return _loginMsg;
    }
    
    void loginMsg(LoginMsg loginMsg)
    {
        _loginMsg = loginMsg;
    }

    void clear()
    {
        _loginMsg = null; 
    }
}
