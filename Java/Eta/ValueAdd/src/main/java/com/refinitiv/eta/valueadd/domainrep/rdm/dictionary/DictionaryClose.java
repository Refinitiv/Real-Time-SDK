/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.dictionary;


/**
 * The RDM Dictionary Close. Used by an OMM Consumer to close a Dictionary
 * stream.
 * 
 * @see DictionaryMsg
 */
public interface DictionaryClose extends DictionaryMsg
{
    /**
     * Performs a deep copy of {@link DictionaryClose} object.
     * 
     * @param destCloseMsg Message to copy dictionary close object into. It cannot be null.
     * 
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(DictionaryClose destCloseMsg);
}