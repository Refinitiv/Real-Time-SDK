/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.dictionary;


/**
 * Factory for RDM dictionary messages.
 * 
 */
public class DictionaryMsgFactory
{
    /**
     * This class is not instantiated
     */  
    private DictionaryMsgFactory()
    {
        throw new AssertionError();
    }
    
    /**
     * Creates a RDMDictionaryMsg that may be cast to any message class defined by
     * {@link DictionaryMsgType} (e.g. {@link DictionaryClose}, {@link DictionaryStatus},
     * {@link DictionaryRequest}, {@link DictionaryRefresh}
     * 
     * @return RDMDictionaryMsg object
     * 
     * @see DictionaryMsg
     * @see DictionaryClose
     * @see DictionaryStatus
     * @see DictionaryRequest
     * @see DictionaryRefresh
     */
    public static DictionaryMsg createMsg()
    {
        return new DictionaryMsgImpl();
    }
}
