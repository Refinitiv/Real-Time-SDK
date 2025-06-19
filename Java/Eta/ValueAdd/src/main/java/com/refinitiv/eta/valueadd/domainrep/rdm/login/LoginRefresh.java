/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.State;

/**
 * The RDM Login Refresh. This message is used to respond to a Login Request
 * message after the user's login is accepted.
 * 
 * @see LoginMsg
 * @see LoginRefreshFlags
 */
public interface LoginRefresh extends LoginMsg
{
    
    /**
     * The RDM Login refresh flags. Populated by {@link LoginRefreshFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags);

    /**
     * The RDM Login refresh flags. Populated by {@link LoginRefreshFlags}.
     * 
     * @return flags
     */
    public int flags();

    /**
     * Performs a deep copy of {@link LoginRefresh} object.
     * 
     * @param destRefreshMsg Message to copy login refresh object into. It
     *            cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(LoginRefresh destRefreshMsg);

    /**
     * Returns userName that was used when sending the Login refresh.
     * 
     * @return - User name buffer. While encoding, this buffer can be populated
     *         with the value to be encoded.
     */
    public Buffer userName();

    /**
     * Sets userName for login to the user specified buffer. Data and position
     * of userName buffer will be set to passed in buffer's data and position.
     * Note that this creates garbage if buffer is backed by String object.
     *
     * @param userName the user name
     */
    public void userName(Buffer userName);

    /**
     * Checks the presence of user name field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if user name is present, false - if not.
     */
    public boolean checkHasUserName();

    /**
     * Applies user name presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     * 
     */
    public void applyHasUserName();

    /**
     * The type of the userName that was used with the Login Refresh. Populated
     * by {@link com.refinitiv.eta.rdm.Login.UserIdTypes}
     * 
     * @return user name type.
     */
    public int userNameType();

    /**
     * The type of the userName that was used with the Login Request.Populated
     * by {@link com.refinitiv.eta.rdm.Login.UserIdTypes}
     * 
     * @param userNameType - user name type.
     */
    public void userNameType(int userNameType);

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
     * Returns the current state of the login stream.
     * 
     * @return state
     */
    public State state();

    /**
     * Sets state - current state of the login stream.
     *
     * @param state the state
     */
    public void state(State state);

    /**
     * sequenceNumber - The sequence number of this message.
     * 
     * @return sequenceNumber
     */
    public long sequenceNumber();

    /**
     * sequenceNumber - The sequence number of this message.
     *
     * @param sequenceNumber the sequence number
     */
    public void sequenceNumber(long sequenceNumber);

    /**
     * Checks the presence of sequenceNumber field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if sequenceNumber field is present, false - if not.
     */
    public boolean checkHasSequenceNumber();

    /**
     * Applies sequenceNumber presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     * 
     */
    public void applyHasSequenceNumber();

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
     * Applies solicited flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applySolicited();

    /**
     * Checks the presence of solicited flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     *
     * @return true, if successful
     */
    public boolean checkSolicited();

    /**
     * Sets login attrib information.
     * 
     * @param attrib -login attrib.
     */
    public void attrib(LoginAttrib attrib);

    /**
     * Returns login attrib information.
     * 
     * @return login attrib.
     */
    public LoginAttrib attrib();

    /**
     * Checks the presence of attrib field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if attrib field is present, false - if not.
     */
    public boolean checkHasAttrib();

    /**
     * Applies attrib presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasAttrib();

    /**
     * Sets the set of features provider of this login refresh message supports.
     * 
     * @param features -set of provided features.
     */
    public void features(LoginSupportFeatures features);

    /**
     * Returns the set of features provider of this login refresh message
     * supports.
     * 
     * @return features -set of provided features.
     */
    public LoginSupportFeatures features();

    /**
     * Checks the presence of features member.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if features member is present, false - if not.
     */
    public boolean checkHasFeatures();

    /**
     * Applies features presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasFeatures();
    
    /**
     * Sets connection config information.
     *
     * @param connectionConfig the connection config
     */
    public void connectionConfig(LoginConnectionConfig connectionConfig);

    /**
     * Returns connection config information.
     * 
     * @return connectionConfig.
     */
    public LoginConnectionConfig connectionConfig();

    /**
     * Checks the presence of connectionConfig member.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if connectionConfig member is present, false - if not.
     */
    public boolean checkHasConnectionConfig();

    /**
     * Applies connectionConfig presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasConnectionConfig();
   
    /**
     * Sets the authentication time to reissue in seconds.
     * 
     * Used with the {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     *
     * @param authenticationTTReissue the authentication TT reissue
     */
    public void authenticationTTReissue(long authenticationTTReissue);

    /**
     * Returns the authentication time to reissue in seconds.
     * 
     * Used with the {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     * 
     * @return authenticationTTReissue
     */
    public long authenticationTTReissue();

    /**
     * Checks the presence of the authentication time to reissue field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if authenticationTTReissue field is present, false - if
     *         not.
     */
    public boolean checkHasAuthenticationTTReissue();

    /**
     * Applies authenticationTTReissue field flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasAuthenticationTTReissue();

    /**
     * Sets the authentication extended response field.
     * 
     * Used with the {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     *
     * @param authenticationExtendedResp the authentication extended resp
     */
    public void authenticationExtendedResp(Buffer authenticationExtendedResp);

    /**
     * Returns the authentication extended data response.
     * 
     * Used with the {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     * 
     * @return authenticationExtendedResp
     */
    public Buffer authenticationExtendedResp();

    /**
     * Checks the presence of the optional authentication extended response
     * field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if authenticationExtendedResp field is present, false - if
     *         not.
     */
    public boolean checkHasAuthenticationExtendedResp();

    /**
     * Applies authenticationExtendedResp field flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasAuthenticationExtendedResp();

    /**
     * Sets the authentication error code.
     * 
     * Used with the {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     *
     * @param authenticationErrorCode the authentication error code
     */
    public void authenticationErrorCode(long authenticationErrorCode);

    /**
     * Returns the authentication error code.
     * 
     * Used with the {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     * 
     * @return authenticationErrorCode
     */
    public long authenticationErrorCode();

    /**
     * Checks the presence of the authentication error code field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if authenticationErrorCode field is present, false - if
     *         not.
     */
    public boolean checkHasAuthenticationErrorCode();

    /**
     * Applies authenticationErrorCode field flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasAuthenticationErrorCode();

    /**
     * Sets the authentication error text.
     * 
     * Used with the {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     *
     * @param authenticationErrorText the authentication error text
     */
    public void authenticationErrorText(Buffer authenticationErrorText);

    /**
     * Returns the authentication error text.
     * 
     * Used with the {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     * login userNameType.
     * 
     * @return authenticationErrorText
     */
    public Buffer authenticationErrorText();

    /**
     * Checks the presence of the authentication error text field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if authenticationErrorText field is present, false - if
     *         not.
     */
    public boolean checkHasAuthenticationErrorText();

    /**
     * Applies authenticationErrorText field flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasAuthenticationErrorText();
}