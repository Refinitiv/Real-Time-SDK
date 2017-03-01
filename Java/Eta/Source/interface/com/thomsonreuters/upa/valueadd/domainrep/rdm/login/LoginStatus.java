package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.State;

/**
 * The RDM Login Status. Used by an OMM Provider to indicate changes to the
 * Login stream.
 * 
 * @see LoginMsg
 */
public interface LoginStatus extends LoginMsg
{
    /**
     * The RDM Login Status flags. Populated by {@link LoginStatusFlags}.
     * 
     * @param flags
     */
    public void flags(int flags);

    /**
     * The RDM Login Status flags. Populated by {@link LoginStatusFlags}.
     * 
     * @return flags
     */
    public int flags();

    /**
     * Performs a deep copy of {@link LoginStatus} object.
     *
     * @param destStatusMsg Message to copy login status object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(LoginStatus destStatusMsg);

    /**
     * userName - The userName that was used when sending the Login Status.
     * 
     * @return - userName
     */
    public Buffer userName();
    
    /**
     * Sets userName for login to the user specified buffer. Data and
     * position of userName buffer will be set to passed in buffer's data and
     * position. Note that this creates garbage if buffer is backed by String
     * object.
     * 
     * @param userName
     */
    public void userName(Buffer userName);

    /**
     * Checks the presence of user name field.
     * 
     * userName - This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if user name is present, false - if not.
     */
    public boolean checkHasUserName();

    /**
     * userName - Applies user name presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasUserName();

    /**
     * nameType - The type of the userName that was used with the Login Status. Populated
     * by {@link com.thomsonreuters.upa.rdm.Login.UserIdTypes}
     * 
     * @return nameType
     */
    public int userNameType();

    /**
     * nameType - The type of the userName that was used with the Login Request.Populated
     * by {@link com.thomsonreuters.upa.rdm.Login.UserIdTypes}
     * 
     * @param nameType
     */
    public void userNameType(int nameType);

    /**
     * Checks the presence of user name type field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if user name type is present, false - if not.
     */
    public boolean checkHasUserNameType();

    /**
     * Applies user name type presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     * 
     */
    public void applyHasUserNameType();

    /**
     * The current state of the login stream.
     * 
     * @return state.
     */
    public State state();

    /**
     * Checks the presence of state field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if state field is present, false - if not.
     */
    public boolean checkHasState();

    
    /**
     * Applies state presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasState();

    /**
     * Checks the presence of clear cache flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkClearCache();

    /**
     * Applies clear cache flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyClearCache();

    /**
     * Sets the authentication error code.
     * 
     * Used with the {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     * 
     * @param authenticationErrorCode
     */
    public void authenticationErrorCode(long authenticationErrorCode);

    /**
     * Returns the authentication error code.
     * 
     * Used with the {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN} login userNameType.
     * 
     * @return authenticationErrorCode
     */
    public long authenticationErrorCode();

    /**
     * Checks the presence of the authentication error code field.
     * 
     * Used with the {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN} login userNameType.
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if authenticationErrorCode field is present, false - if
     *         not.
     */
    public boolean checkHasAuthenticationErrorCode();

    /**
     * Applies authenticationErrorCode field flag.
     * 
     * Used with the {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN} login userNameType.
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasAuthenticationErrorCode();

    /**
     * Sets the authentication error text.
     * 
     * Used with the {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN} login userNameType.
     * 
     * @param authenticationErrorText
     */
    public void authenticationErrorText(Buffer authenticationErrorText);

    /**
     * Returns the authentication error text.
     * 
     * Used with the {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN} login userNameType.
     * 
     * @return authenticationErrorText
     */
    public Buffer authenticationErrorText();

    /**
     * Checks the presence of the authentication error text field.
     * 
     * Used with the {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN} login userNameType.
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if authenticationErrorText field is present, false - if
     *         not.
     */
    public boolean checkHasAuthenticationErrorText();

    /**
     * Applies authenticationErrorText field flag.
     * 
     * Used with the {@link com.thomsonreuters.upa.rdm.ElementNames#AUTHN_TOKEN} login userNameType.
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasAuthenticationErrorText();
}