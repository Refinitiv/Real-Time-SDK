/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * A combination of bit values that indicate special behaviors and the presence of optional content.
 * 
 * @see UpdateMsg
 */
public class UpdateMsgFlags
{
    /**
     * This class is not instantiated
     */
    private UpdateMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x000) No Flags set */
    public static final int NONE = 0x000;

    /** (0x001) Indicates that Update Message has Extended Header */
    public static final int HAS_EXTENDED_HEADER = 0x001;

    /** (0x002) Indicates that Update Message has Permission Expression */
    public static final int HAS_PERM_DATA = 0x002;

    /** (0x008) Indicates that Update Message has Update Key */
    public static final int HAS_MSG_KEY = 0x008;

    /** (0x010) Indicates that Update Message has Sequence Number */
    public static final int HAS_SEQ_NUM = 0x010;

    /** (0x020) Indicates that Update Message contains conflation information */
    public static final int HAS_CONF_INFO = 0x020;

    /** (0x040) Indicates that Message is transient and should not be cached */
    public static final int DO_NOT_CACHE = 0x040;

    /** (0x080) Indicates that Update Message should not be conflated */
    public static final int DO_NOT_CONFLATE = 0x080;

    /** (0x100) Indicates that Update message should not ripple */
    public static final int DO_NOT_RIPPLE = 0x100;

    /**
     * (0x200) Indicates that this data was posted by the user with this
     * identifying information
     */
    public static final int HAS_POST_USER_INFO = 0x200;

    /**
     * (0x400) Indicates that this update can be discarded. Common for options
     * with no open interest
     */
    public static final int DISCARDABLE = 0x400;
}
