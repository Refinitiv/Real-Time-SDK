package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;

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
