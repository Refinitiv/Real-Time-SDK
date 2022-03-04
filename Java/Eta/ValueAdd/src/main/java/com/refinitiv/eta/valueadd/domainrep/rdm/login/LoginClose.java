/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

/**
 * The RDM Login Close. Used by an OMM Consumer or OMM Non-Interactive Provider to close a Login.
 * stream.
 * 
 * @see LoginMsg
 */
public interface LoginClose extends LoginMsg
{
    /**
     * Performs a deep copy of {@link LoginClose} object.
     *
     * @param destCloseMsg Message to copy login close object into. It cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(LoginClose destCloseMsg);
}