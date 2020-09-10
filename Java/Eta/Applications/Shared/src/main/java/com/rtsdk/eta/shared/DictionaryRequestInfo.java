package com.rtsdk.eta.shared;

import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;

/**
 * Dictionary request information.
 */
public class DictionaryRequestInfo
{
    public DictionaryRequest dictionaryRequest;
    public Channel channel;
    public boolean isInUse;
    
    public DictionaryRequestInfo()
    {
        dictionaryRequest = (DictionaryRequest) DictionaryMsgFactory.createMsg(); 
        dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        channel = null;
        isInUse = false;
    }

    public void clear()
    {
        dictionaryRequest.clear();
        channel = null;
        isInUse = false;
    }
    
    public DictionaryRequest dictionaryRequest()
    {
    	return dictionaryRequest;
    }
    
    public Channel channel()
    {
    	return channel;
    }
    
    public boolean isInUse()
    {
    	return isInUse;
    }
}
