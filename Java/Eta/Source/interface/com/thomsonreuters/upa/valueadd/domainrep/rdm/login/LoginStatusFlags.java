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
    
    /** (0x0010) Indicates presence of the authenticationErrorCode member. 
     * This is used when the userNameType member is set to
     * {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN}
     */
    public static final int HAS_AUTHENTICATION_ERROR_CODE = 0x0010;

    /** (0x0020) Indicates presence of the authenticationErrorText member. 
     * This is used when the userNameType member is set to
     * {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN}
     */
    public static final int HAS_AUTHENTICATION_ERROR_TEXT = 0x0020;

  
    private LoginStatusFlags()
    {
        throw new AssertionError();
    }
}