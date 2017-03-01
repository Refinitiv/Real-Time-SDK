package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

/**
 * The RDM login refresh flags.
 * 
 * @see LoginRefresh
 */
public class LoginRefreshFlags
{
    /** (0x00000) No flags set. */
    public static final int NONE = 0x0000;

    /**
     * (0x0001) Indicates whether the receiver of the refresh should clear any
     * associated cache information.
     */
    public static final int CLEAR_CACHE = 0x0001;

    /**
     * (0x0002) Indicates presence of connectionconfig - payload for login
     * refresh containing numStandbyServers and serverList members.
     */
    public static final int HAS_CONN_CONFIG = 0x0002;

    /** (0x0004) Indicates presence of login attrib member. */
    public static final int HAS_ATTRIB = 0x0004;

    /** (0x0008) Indicates presence of the sequenceNumber member. */
    public static final int HAS_SEQ_NUM = 0x0008;

    /** (0x0010) Indicates presence of login support features member. */
    public static final int HAS_FEATURES = 0x0010;

    /** (0x0020) Indicates presence of the userName member. */
    public static final int HAS_USERNAME = 0x0020;

    /** (0x0004) Indicates presence of the userNameType member. */
    public static final int HAS_USERNAME_TYPE = 0x0040;

    /**
     * (0x0008) Indicates whether this refresh is being provided in response to
     * a request.
     */
    public static final int SOLICITED = 0x0080;

    //Reserve space for this flag
    //public static final int HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD = 0x0100; 

    /** (0x0200) Indicates presence of the authenticationTTReissue member.
    * This is used when the userNameType member is set to
    * {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN}
    */
    public static final int HAS_AUTHENTICATION_TT_REISSUE = 0x0200;

    /** (0x0400) Indicates presence of the authenticationExtendedResp member.
     * This is optionally used when the userNameType member is set to
     * {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN}
     */
    public static final int HAS_AUTHENTICATION_EXTENDED_RESP = 0x0400;

    /** (0x0800) Indicates presence of the authenticationErrorCode member. 
     * This is used when the userNameType member is set to
     * {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN}
     */
    public static final int HAS_AUTHENTICATION_ERROR_CODE = 0x0800;

    /** (0x1000) Indicates presence of the authenticationErrorText member. 
     * This is used when the userNameType member is set to
     * {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN}
     */
    public static final int HAS_AUTHENTICATION_ERROR_TEXT = 0x1000;
    
    private LoginRefreshFlags()
    {
        throw new AssertionError();
    }
}