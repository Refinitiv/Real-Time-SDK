/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.*;

/**
 * Warm Standby Information for the RDM Login Consumer Connection Status.
 *
 * @see LoginConsumerConnectionStatus
 */
public interface LoginWarmStandbyInfo
{
    /**
     *
     * The desired Warm Standby Mode. Populated by {@link com.refinitiv.eta.rdm.Login.ServerTypes}.
     * 
     * @return warmStandbyMode
     */
    public long warmStandbyMode();

    /**
     * The desired Warm Standby Mode. Populated by {@link com.refinitiv.eta.rdm.Login.ServerTypes}.
     *
     * @param warmStandbyMode the warm standby mode
     */
    public void warmStandbyMode(long warmStandbyMode);

    /**
     * Action associated with this information.
     * 
     * @return action
     */
    public int action();

    /**
     * Action associated with this information.
     *
     * @param action the action
     */
    public void action(int action);
    
    /**
    * Clears a {@link LoginWarmStandbyInfo}.
    * @see LoginConsumerConnectionStatus
    */
    public void clear();

    /**
     * Encodes a {@link LoginWarmStandbyInfo}.
     * 
     * @param encodeIter The Encode Iterator
     * 
     * @return ETA return value.
     * 
     */
    public int encode(EncodeIterator encodeIter);

    /**
     * Decodes a {@link LoginWarmStandbyInfo}.
     *
     * @param dIter The Decode Iterator
     * @param msg the msg
     * @return ETA return value
     */
    public int decode(DecodeIterator dIter, Msg msg);

    /**
     * Performs a deep copy of a {@link LoginWarmStandbyInfo} object.
     * 
     * @param destWarmStandbyInfo The resulting copy of the RDM Login WarmStandby Info
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(LoginWarmStandbyInfo destWarmStandbyInfo);
}