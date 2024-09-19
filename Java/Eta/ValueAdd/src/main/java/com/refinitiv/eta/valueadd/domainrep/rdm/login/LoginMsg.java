/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;

/**
 * The RDM Login Base Message. This RDM dictionary messages may be reused or
 * pooled in a single collection via their common {@link LoginMsg} interface
 * and re-used as a different {@link LoginMsgType}. RDMDictionaryMsgType
 * member may be set to one of these to indicate the specific RDMDictionaryMsg
 * class.
 * 
 * @see LoginClose
 * @see LoginRefresh
 * @see LoginRequest
 * @see LoginStatus
 * @see LoginAck
 * @see LoginPost
 * @see LoginConsumerConnectionStatus
 * 
 * @see LoginMsgFactory - Factory for creating RDM login messages
 * 
 * @see LoginMsgType
 */
public interface LoginMsg extends MsgBase
{
    /**
     * Login message type. These are defined per-message class basis for login
     * domain.
     * 
     * @see LoginClose
     * @see LoginRefresh
     * @see LoginRequest
     * @see LoginStatus
     * @see LoginAck
     * @see LoginPost
     * @see LoginConsumerConnectionStatus
     * 
     * @return login message type.
     */
    public LoginMsgType rdmMsgType();

    /**
     * Login message type. These are defined per-message class basis for
     * login domain.
     * 
     * @see LoginClose
     * @see LoginRefresh
     * @see LoginRequest
     * @see LoginStatus
     * @see LoginAck
     * @see LoginPost
     * @see LoginConsumerConnectionStatus
     * 
     * @param rdmLoginMsgType - login message type.
     */
    public void rdmMsgType(LoginMsgType rdmLoginMsgType);
}
