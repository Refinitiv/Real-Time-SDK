/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Combination of bit values to indicate the presence of optional {@link MsgKey} members.
 * 
 * @see MsgKey
 */
public class MsgKeyFlags
{
    /**
     * This class is not instantiated
     */
    private MsgKeyFlags()
    {
        throw new AssertionError();
    }

    /** (0x0000) No Key Flags */
    public static final int NONE = 0x0000;

    /** (0x0001) Indicates the presence of the serviceId field. */
    public static final int HAS_SERVICE_ID = 0x0001;

    /** (0x0002) Indicates the presence of the name field. */
    public static final int HAS_NAME = 0x0002;

    /** (0x0004) Indicates the presence of the nameType field. */
    public static final int HAS_NAME_TYPE = 0x0004;
    
    /** (0x0008) Indicates the presence of the filter field. */
    public static final int HAS_FILTER = 0x0008;
    
    /** (0x0010) Indicates the presence of the identifier field. */
    public static final int HAS_IDENTIFIER = 0x0010;
    
    /**
     * (0x0020) Indicates key has attribute information, this includes msgKey.attrib and
     * msgKey.attribContainerType
     */
    public static final int HAS_ATTRIB = 0x0020;
}
