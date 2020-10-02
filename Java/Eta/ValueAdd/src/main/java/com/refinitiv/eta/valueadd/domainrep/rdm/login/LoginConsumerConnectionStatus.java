package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;

/**
 * The RDM Login Consumer Connection Status. Used by an OMM Consumer to send
 * Warm Standby information.
 * 
 * @see MsgBase
 * @see LoginWarmStandbyInfo
 */
public interface LoginConsumerConnectionStatus extends LoginMsg
{
    /**
     * The Consumer Connection Status flags. Populated by
     * {@link LoginConsumerConnectionStatusFlags}.
     * 
     * @return flags
     */
    public int flags();

    /**
     * The Consumer Connection Status flags. Populated by
     * {@link LoginConsumerConnectionStatusFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags);

    /**
     * Checks the presence of Warm Standby Information.
     * 
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @return true if warm standby info is present. If not, returns false.
     */
    public boolean checkHasWarmStandbyInfo();

    /**
     * Checks the presence of Warm Standby Information.
     * 
     * Flags may also be bulk-set via {@link #flags()}.
     */
    public void applyHasWarmStandbyInfo();

    /**
     * Warm Standby Information. Presence indicated by
     * {@link LoginConsumerConnectionStatusFlags#HAS_WARM_STANDBY_INFO}.
     * 
     * @return Warm standby information. While encoding, returned object may be
     *         populated with the value to be encoded.
     * 
     */
    public LoginWarmStandbyInfo warmStandbyInfo();
    
    /**
     * Warm Standby Information. Presence indicated by
     * {@link LoginConsumerConnectionStatusFlags#HAS_WARM_STANDBY_INFO}.
     * 
     * @param warmStandbyInfo -Warm standby information.
     * 
     */
    public void warmStandbyInfo(LoginWarmStandbyInfo warmStandbyInfo);


    /**
     * Performs a deep copy of {@link LoginConsumerConnectionStatus} object.
     *
     * @param destConnStatusMsg Message to copy login consumer connection status object into. It cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(LoginConsumerConnectionStatus destConnStatusMsg);
}