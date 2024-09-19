/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;



/**
 * Factory for RDM login messages.
 */
public class LoginMsgFactory
{
    /**
     * This class is not instantiated
     */
    private LoginMsgFactory()
    {
        throw new AssertionError();
    }

    /**
     * Creates a RDMLoginMsg that may be cast to any message class defined by
     * {@link LoginMsgType} (e.g. {@link LoginClose},
     * {@link LoginStatus}, {@link LoginRequest}, {@link LoginRefresh}
     * 
     * @return RDMLoginMsg object
     * 
     * @see LoginClose
     * @see LoginRefresh
     * @see LoginRequest
     * @see LoginStatus
     * @see LoginAck
     * @see LoginPost
     * @see LoginConsumerConnectionStatus
     */
    public static LoginMsg createMsg()
    {
        return new LoginMsgImpl();
    }
    

    /**
     * Creates {@link LoginWarmStandbyInfo}.
     * 
     * @return LoginWarmStandbyInfo object
     * 
     * @see LoginWarmStandbyInfo
     */
    public static LoginWarmStandbyInfo createWarmStandbyInfo()
    {
        return new LoginWarmStandbyInfoImpl();
    }
    
    /**
     * Creates {@link LoginConnectionConfig}.
     * 
     * @return ConnectionConfig object
     * 
     * @see LoginConnectionConfig
     */
    public static LoginConnectionConfig createConnectionConfig()
    {
        return new LoginConnectionConfigImpl();
    }
}
