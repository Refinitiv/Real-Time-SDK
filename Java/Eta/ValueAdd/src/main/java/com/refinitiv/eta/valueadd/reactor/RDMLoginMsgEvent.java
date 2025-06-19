/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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
