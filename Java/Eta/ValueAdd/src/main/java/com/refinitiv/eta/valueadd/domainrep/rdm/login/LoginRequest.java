/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;

/**
 * The RDM login request. Used by an OMM Consumer or OMM Non-Interactive
 * Provider to request a login.
 * 
 * @see MsgBase
 * @see LoginMsg
 */
public interface LoginRequest extends LoginMsg
{
    
    /**
     * The RDM Login request flags. Populated by {@link LoginRequestFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags);

    /**
     * The RDM Login request flags. Populated by {@link LoginRequestFlags}.
     * 
     * @return flags
     */
    public int flags();

    /**
     * Performs a deep copy of {@link LoginRequest} object.
     * 
     * @param destRequestMsg Message to copy login request object into. It
     *            cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(LoginRequest destRequestMsg);

    /**
     * Initializes a LoginRequest, clearing it and filling in a typical
     * userName, applicationName and position.
     *
     * @param streamId the stream id
     */
    public void initDefaultRequest(int streamId);

    /**
     * The userName that was used when sending the Login Request.
     * 
     * @return - User name buffer.
     */
    public Buffer userName();

    /**
     * Sets userName for login to the user specified buffer. Data and position
     * of serviceName buffer will be set to passed in buffer's data and
     * position. For UserAuthn Authentication, this should contain the Authentication
     * Token. Note that this creates garbage if buffer is backed by String
     * object.
     *
     * @param userName the user name
     */
    public void userName(Buffer userName);

    /**
     * userNameType - The type of the userName that was used with the Login
     * Refresh.Populated by {@link com.refinitiv.eta.rdm.Login.UserIdTypes}
     * 
     * @return userNameType
     */
    public int userNameType();

    /**
     * userNameType - The type of the userName that was used with the Login
     * Request. Populated by
     * {@link com.refinitiv.eta.rdm.Login.UserIdTypes}
     *
     * @param userNameType the user name type
     */
    public void userNameType(int userNameType);

    /**
     * Checks the presence of user name type.
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
     * Checks if login request is pause.
     * 
     * Used by a Consumer to request that all open items on a channel be paused.
     * 
     * @return true - if request has pause flag set, false - if pause flag is
     *         not set.
     */
    public boolean checkPause();

    /**
     * Applies pause flag to login request.
     * 
     * Used by a Consumer to request that all open items on a channel be paused.
     */
    public void applyPause();

    /**
     * Checks if no refresh required flag is set.
     * 
     * No Refresh flag indicates the Consumer or Non-Interactive provider does
     * not require a refresh.
     * 
     * @return true - NoRefresh request is true.
     */
    public boolean checkNoRefresh();

    /**
     * Applies no refresh required flag.
     * 
     * No Refresh flag indicates the Consumer or Non-Interactive provider does
     * not require a refresh.
     */
    public void applyNoRefresh();

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
     * downloadConnectionConfig - Indicates whether the Consumer desires
     * connection information. If available, a list of servers will be present
     * in the serverList field of the {@link LoginRefresh}.
     * 
     * @return downloadConnectionConfig
     */
    public long downloadConnectionConfig();

    /**
     * downloadConnectionConfig - Indicates whether the Consumer desires
     * connection information. If available, a list of servers will be present
     * in the serverList field of the {@link LoginRefresh}.
     *
     * @param downloadConnectionConfig the download connection config
     */
    public void downloadConnectionConfig(long downloadConnectionConfig);

    /**
     * Checks the presence of download connection config field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if download connection config field is present, false - if
     *         not.
     */
    public boolean checkHasDownloadConnectionConfig();

    /**
     * Applies download connection config presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     * 
     */
    public void applyHasDownloadConnectionConfig();

    /**
     * Returns instance id. InstanceId can be used to differentiate applications
     * running on the same machine.
     * 
     * @return instanceId
     */
    public Buffer instanceId();

    /**
     * Sets instance id. InstanceId can be used to differentiate applications
     * running on the same machine. Note that this creates garbage if buffer is
     * backed by String object.
     *
     * @param instanceId the instance id
     */
    public void instanceId(Buffer instanceId);

    /**
     * Checks the presence of instance id field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if instance id field is present, false - if not.
     */
    public boolean checkHasInstanceId();

    /**
     * Applies instance id presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     * 
     */
    public void applyHasInstanceId();

    /**
     * The password.
     * 
     * @return password
     */
    public Buffer password();

    /**
     * Checks the presence of password field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if password is present, false - if not.
     */
    public boolean checkHasPassword();

    /**
     * Applies password presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasPassword();

    /**
     * Indicates the role of the application. Populated by
     * {@link com.refinitiv.eta.rdm.Login.RoleTypes}
     * 
     * @return role of the application.
     */
    public long role();

    /**
     * Indicates the role of the application. Populated by
     * {@link com.refinitiv.eta.rdm.Login.RoleTypes}
     * 
     * @param role of the application.
     */
    public void role(long role);

    /**
     * Checks the presence of application role field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if application role field is present, false - if not.
     */
    public boolean checkHasRole();

    /**
     * Applies application role field flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasRole();

    /**
     * Gets the authentication extended data.
     * 
     * optionally used with the
     * {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN} login
     * userNameType.
     * 
     * @return authenticationExtended
     */
    public Buffer authenticationExtended();

    /**
     * Sets the authentication extended data.
     * 
     * Optionally used with the
     * {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN} login
     * userNameType.
     *
     * @param authenticationExtended the authentication extended
     */
    public void authenticationExtended(Buffer authenticationExtended);

    /**
     * Checks the presence of the authenticationExtended data field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if authenticationExtended data field is present, false -
     *         if not.
     */
    public boolean checkHasAuthenticationExtended();

    /**
     * Applies authentication extendedData field flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasAuthenticationExtended();

}
