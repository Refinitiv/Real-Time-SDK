/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;

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
