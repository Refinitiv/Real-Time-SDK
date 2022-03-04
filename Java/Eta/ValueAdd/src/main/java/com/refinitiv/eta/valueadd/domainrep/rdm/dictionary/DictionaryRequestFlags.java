/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.dictionary;

/**
 * The RDM Dictionary Request Flags.
 * 
 * @see DictionaryRequest
 */
public class DictionaryRequestFlags
{
    /**
     * (0x0) No flags set.
     */
    public static final int NONE = 0x00; 
    
    
    /**
     * (0x1) Indicates this is to be a streaming request.
     */
    public static final int STREAMING = 0x01;

    private DictionaryRequestFlags()
    {
        throw new AssertionError();
    }
}
