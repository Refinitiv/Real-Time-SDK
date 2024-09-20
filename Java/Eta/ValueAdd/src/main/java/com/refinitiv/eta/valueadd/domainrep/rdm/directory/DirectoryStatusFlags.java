/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

/**
 * The RDM Directory Status Flags.
 * 
 * @see DirectoryStatus
 */
public class DirectoryStatusFlags
{
    /** (0x00) No flags set. */
    public static final int NONE = 0x00;              
    
    /** (0x01) Indicates presence of the filter member. */
    public static final int HAS_FILTER = 0x01;         
    
    /** (0x02) Indicates presence of the serviceId member. */
    public static final int HAS_SERVICE_ID = 0x02;
    
    /** (0x04) Indicates presence of the state member. */
    public static final int HAS_STATE = 0x04;

    /** (0x08) Indicates whether the receiver of the directory status should clear any
     * associated cache information.
     */
    public static final int CLEAR_CACHE = 0x08;

    private DirectoryStatusFlags()
    {
        throw new AssertionError();
    }
}