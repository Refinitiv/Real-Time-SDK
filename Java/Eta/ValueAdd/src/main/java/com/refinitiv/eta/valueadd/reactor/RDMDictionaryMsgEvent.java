/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;

/**
 * Event provided to RDMDictionaryMsgCallback methods.
 * 
 * @see ReactorMsgEvent
 */
public class RDMDictionaryMsgEvent extends ReactorMsgEvent
{
    DictionaryMsg _dictionaryMsg;

    RDMDictionaryMsgEvent()
    {
        super();
    }

    void rdmDictionaryMsg(DictionaryMsg rdmDictionaryMsg)
    {
        _dictionaryMsg = rdmDictionaryMsg;
    }

    /**
     * The DictionaryMsg associated with this message event.
     * 
     * @return DictionaryMsg
     */
    public DictionaryMsg rdmDictionaryMsg()
    {
        return _dictionaryMsg;
    }
}
