package com.thomsonreuters.upa.examples.provider;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;

/**
 * Dictionary request information.
 */
public class DictionaryRequestInfo
{
    DictionaryRequest dictionaryRequest;
    Channel channel;
    boolean isInUse;
    
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
