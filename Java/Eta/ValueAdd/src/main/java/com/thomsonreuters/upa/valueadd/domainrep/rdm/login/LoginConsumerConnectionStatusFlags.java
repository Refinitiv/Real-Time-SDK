package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

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