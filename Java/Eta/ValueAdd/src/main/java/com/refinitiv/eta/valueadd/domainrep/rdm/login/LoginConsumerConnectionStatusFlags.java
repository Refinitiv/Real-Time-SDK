/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

/**
 * RDM Login Consumer Connection Status flags.
 * 
 * @see LoginConsumerConnectionStatus
 */
public class LoginConsumerConnectionStatusFlags
{
    /** (0x00) No flags set. */
    public static final int NONE = 0x00;
    
    /**
     * (0x01) Indicates presence of Warm Standby information. 
     */
    public static final int HAS_WARM_STANDBY_INFO = 0x01;  
    
    private LoginConsumerConnectionStatusFlags()
    {
    }
}