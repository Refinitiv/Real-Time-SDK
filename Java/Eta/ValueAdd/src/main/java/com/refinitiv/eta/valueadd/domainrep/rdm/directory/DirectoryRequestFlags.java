/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;

/**
 * The RDM Directory Request Flags.
 * 
 * @see DirectoryRequest
 */
public class DirectoryRequestFlags
{
    /** (0x00) No flags set. */
    public static final int NONE = 0x00;

    /** (0x01) Indicates presence of the serviceId member. */
    public static final int STREAMING = 0x01;

    /**
     * (0x02) Indicates whether the request is a streaming request, i.e. whether
     * updates about source directory information are desired.
     */
    public static final int HAS_SERVICE_ID = 0x02;

    private DirectoryRequestFlags()
    {
        throw new AssertionError();
    }
}