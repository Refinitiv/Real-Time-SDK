/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;

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
