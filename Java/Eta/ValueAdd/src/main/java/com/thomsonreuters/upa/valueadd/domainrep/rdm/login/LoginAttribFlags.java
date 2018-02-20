package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

/**
 * The RDM login attrib Flags. 
 * 
 * @see LoginAttrib
 */
public class LoginAttribFlags
{
    /** (0x0000) No flags set */
    public static final int NONE = 0x0000;

    /** (0x0001) Indicates presence of the allowSuspectData member. */
    public static final int HAS_ALLOW_SUSPECT_DATA = 0x0001;

    /** (0x0002) Indicates presence of the applicationId member */
    public static final int HAS_APPLICATION_ID = 0x0002;

    /** (0x0004) Indicates presence of the applicationName member */
    public static final int HAS_APPLICATION_NAME = 0x0004;

    /** (0x0008) Indicates presence of the position member */
    public static final int HAS_POSITION = 0x0008;

    /** (0x0020) Indicates presence of the providePermissionExpressions member */
    public static final int HAS_PROVIDE_PERM_EXPR = 0x0020;

    /** (0x0040) Indicates presence of the providePermissionProfile member */
    public static final int HAS_PROVIDE_PERM_PROFILE = 0x0040;

    /** (0x0080) Indicates presence of the singleOpen member */
    public static final int HAS_SINGLE_OPEN = 0x0080;

    /**
     * (0x0100) Inform a Provider that it can request dictionary.
     * Support for this request is indicated by the
     * supportProviderDictionaryDownload member of the {@link LoginAttrib}
     */
    public static final int HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD = 0x0100;
    
    private LoginAttribFlags()
    {
        throw new AssertionError();
    }
}