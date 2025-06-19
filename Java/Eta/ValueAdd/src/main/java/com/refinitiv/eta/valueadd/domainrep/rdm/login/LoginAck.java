/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;




/**
 * The RDM Login Ack.  Used for an off-stream Post Acknowledgment
 * 
 * @see LoginMsg
 * 
 * @deprecated use {@link com.refinitiv.eta.codec.AckMsg} instead
 */
@Deprecated
public interface LoginAck extends LoginMsg
{
    /**
     * Performs a deep copy of {@link LoginAck} object.
     *
     * @param destAckMsg Message to copy login ack object into. It cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(LoginAck destAckMsg);
}