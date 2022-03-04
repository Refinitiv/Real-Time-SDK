/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

/**
 * The RDM login request flags.
 * 
 * @see LoginRequest
 */
public class LoginRequestFlags
{
    /** (0x0000) No flags set */
    public static final int NONE = 0x0000;

    /** (0x0001) Indicates presence of the attrib member. */
    public static final int HAS_ATTRIB = 0x0001;

    /** (0x0002) Indicates presence of the downloadConnectionConfig member */
    public static final int HAS_DOWNLOAD_CONN_CONFIG = 0x0002;

    /** (0x0004) Indicates presence of the instanceId member */
    public static final int HAS_INSTANCE_ID = 0x0004;

    /** (0x0008) Indicates presence of the password member */
    public static final int HAS_PASSWORD = 0x0008;

    /** (0x0010) Indicates presence of the role member */
    public static final int HAS_ROLE = 0x0010;

    /** (0x0020) Indicates presence of the userNameType member */
    public static final int HAS_USERNAME_TYPE = 0x0020;

    /**
     * (0x0040) Indicates the Consumer or Non-Interactive provider does not
     * require a refresh.
     */
    public static final int NO_REFRESH = 0x0040;

    /**
     * (0x0080) Used by a Consumer to request that all open items on a channel
     * be paused. Support for this request is indicated by the
     * supportOptimizedPauseResume member of the {@link LoginRefresh}
     */
    public static final int PAUSE_ALL = 0x0080;
        
    /** (0x0200) Indicates presence of the authentication extended data member 
     * This is optionally used when the userNameType member is set to
     * {@link com.refinitiv.eta.rdm.ElementNames#AUTHN_TOKEN}
     */
    public static final int HAS_AUTHENTICATION_EXTENDED = 0x0200;
    
    private LoginRequestFlags()
    {
        throw new AssertionError();
    }
}