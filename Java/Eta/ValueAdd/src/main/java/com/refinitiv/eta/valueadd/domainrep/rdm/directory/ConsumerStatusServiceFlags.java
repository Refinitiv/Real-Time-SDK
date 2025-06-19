/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

/**
 * The RDM Consumer Status Service Flags.
 * 
 * @see DirectoryStatus
 */
public class ConsumerStatusServiceFlags
{
    /** (0x00) No flags set. */
    public static final int NONE = 0x00;              
    
    /** (0x01) Indicates presence of the filter member. */
    public static final int HAS_SOURCE_MIRRORING_MODE = 0x01;         
    
    /** (0x02) Indicates presence of the serviceId member. */
    public static final int HAS_WARM_STANDY_MODE = 0x02;

    private ConsumerStatusServiceFlags()
    {
        throw new AssertionError();
    }
}

