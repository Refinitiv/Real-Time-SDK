/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.directory;


/**
 * The RDM Directory Close.  Used by an OMM Consumer to close an open Source Directory stream.
 * 
 * @see DirectoryMsg
 */
public interface DirectoryClose extends DirectoryMsg
{
    /**
     * Performs a deep copy of {@link DirectoryClose} object.
     *
     * @param destCloseMsg Message to copy directory close object into. It cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(DirectoryClose destCloseMsg);
}