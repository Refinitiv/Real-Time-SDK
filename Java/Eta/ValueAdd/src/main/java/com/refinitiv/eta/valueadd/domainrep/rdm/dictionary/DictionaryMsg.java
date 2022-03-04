/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.dictionary;

import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;

/**
 * The RDM Dictionary Base Message. This RDM dictionary messages may be reused or pooled
 * in a single collection via their common {@link DictionaryMsg} interface
 * and re-used as a different {@link DictionaryMsgType}. RDMDictionaryMsgType
 * member may be set to one of these to indicate the specific
 * RDMDictionaryMsg class.
 * 
 * @see DictionaryClose
 * @see DictionaryRefresh
 * @see DictionaryRequest
 * @see DictionaryStatus
 * 
 * @see DictionaryMsgFactory - Factory for creating RDM dictionary messages
 * 
 * @see DictionaryMsgType
 */
public interface DictionaryMsg extends MsgBase
{
    /**
     * Dictionary message type. These are defined per-message class basis for
     * dictionary domain.
     * 
     * @see DictionaryClose
     * @see DictionaryRefresh
     * @see DictionaryRequest
     * @see DictionaryStatus
     * 
     * @return RDMDictionaryMsgType - dictionary message type.
     */
    public DictionaryMsgType rdmMsgType();

    /**
     * Dictionary message type. These are defined per-message class basis for
     * dictionary domain.
     * 
     * @see DictionaryClose
     * @see DictionaryRefresh
     * @see DictionaryRequest
     * @see DictionaryStatus
     * 
     * @param rdmDictionaryMsgType - dictionary message type.
     */
    public void rdmMsgType(DictionaryMsgType rdmDictionaryMsgType);
}