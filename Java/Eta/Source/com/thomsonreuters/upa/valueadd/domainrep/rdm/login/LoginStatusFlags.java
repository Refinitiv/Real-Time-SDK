package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

/**
 * The RDM Login Status Flags
 * 
 * @see LoginStatus
 */
public class LoginStatusFlags
{
    /** (0x00) No flags set. */
    public static final int NONE = 0x00;
   
    /** (0x01) Indicates presence of the state member. */
    public static final int HAS_STATE = 0x01;
    
    /** (0x02) Indicates presence of the userName member. */
    public static final int HAS_USERNAME = 0x02;
    
    /** (0x04) Indicates presence of the userNameType member. */
    public static final int HAS_USERNAME_TYPE = 0x04;
    
    /**
     * (0x08) Indicates whether the receiver of the login status should clear any
     * associated cache information.
     */
    public static final int CLEAR_CACHE = 0x08;
    
    private LoginStatusFlags()
    {
        throw new AssertionError();
    }
}