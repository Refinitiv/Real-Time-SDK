/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

/**
 * The types of RDM login Messages. rdmMsgType member in {@link LoginMsg} may
 * be set to one of these to indicate the specific RDMLoginMsg class.
 * 
 * @see LoginClose
 * @see LoginRefresh
 * @see LoginRequest
 * @see LoginStatus
 * @see LoginAck
 * @see LoginPost
 * @see LoginConsumerConnectionStatus
 */
public enum LoginMsgType
{
    /**
     * (0) Unknown
     */
    UNKNOWN(0),

    /** (1) Login Request */
    REQUEST(1),
    
    /** (2) Login Close */
    CLOSE(2), 
    
    /** (3) Login Consumer Connection Status */
    CONSUMER_CONNECTION_STATUS(3), 
    
    /** (4) Login Refresh */
    REFRESH(4), 
    
    /** (5) Login Status */
    STATUS(5), 
    
    /** (6) Indicates an off-stream Post Message. */
    @Deprecated
    POST(6),
    
    /** (7) Indicates an off-stream Ack Message. */
    @Deprecated
    ACK(7),

    /** Indicates that a message is an RTT message */
    RTT(8);

    private LoginMsgType(int value)
    {
        this.value = value;
    }

    @SuppressWarnings("unused")
    private int value;
}
