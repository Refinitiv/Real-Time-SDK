/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

/**
 * The RDM Directory Update Flags.
 * 
 * @see DirectoryUpdate
 */
public class DirectoryUpdateFlags
{
    /** (0x00) No flags set. */
    public static final int NONE =   0x00;
    
    /** (0x01) Indicates presence of the serviceId member. */
    public static final int HAS_SERVICE_ID = 0x01;
    
    /** (0x02) Indicates presence of the filter member. */
    public static final int HAS_FILTER = 0x02; 

    /** (0x04) Indicates presence of the sequenceNumber member. */
    public static final int HAS_SEQ_NUM = 0x04;
    
    private DirectoryUpdateFlags()
    {
        throw new AssertionError();
    }
}