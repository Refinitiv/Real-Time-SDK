/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;

/**
* Dictionary request information list.
*/
public class DictionaryRequestInfoList implements Iterable<DictionaryRequestInfo>
{
    private static final int MAX_DICTIONARY_REQUESTS_PER_CLIENT = 2;
    private static final int MAX_DICTIONARY_REQUESTS = (MAX_DICTIONARY_REQUESTS_PER_CLIENT * ProviderSession.NUM_CLIENT_SESSIONS);

    private List<DictionaryRequestInfo> dictionaryRequestInfoList = new ArrayList<DictionaryRequestInfo>(MAX_DICTIONARY_REQUESTS);

    public DictionaryRequestInfoList()
    {
        for (int i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
        {
            dictionaryRequestInfoList.add(new DictionaryRequestInfo());
        }
    }
    
    public void init()
    {
        for (DictionaryRequestInfo dictionaryRequestInfo : dictionaryRequestInfoList)
        {
            dictionaryRequestInfo.clear();
        }
    }
    
    /**
     * Retrieves a dictionary request information object for a channel.
     * 
     * @param chnl The channel to get the dictionary request information
     *             structure for msg
     * @param dictionaryRequest - Dictionary request
     * @return dictionary request information for the channel to use.
     */
    public DictionaryRequestInfo get(Channel chnl, DictionaryRequest dictionaryRequest)
    {
        DictionaryRequestInfo dictionaryRequestInfo = null;

        /* first check if one already in use for this channel and stream id */
        for (DictionaryRequestInfo dictRequestInfo : dictionaryRequestInfoList)
        {
            if (dictRequestInfo.isInUse &&
                    dictRequestInfo.channel == chnl &&
                    dictRequestInfo.dictionaryRequest.streamId() == dictionaryRequest.streamId())
            {
                /*
                 * if dictionary name is different from last request, this is an
                 * invalid request
                 */
                dictionaryRequestInfo = dictRequestInfo;
                if (!dictionaryRequest.dictionaryName().equals(dictRequestInfo.dictionaryRequest.dictionaryName()))
                {
                    return null;
                }
                break;
            }
        }

        /* get a new one if one is not already in use */
        if (dictionaryRequestInfo == null)
        {
            for (DictionaryRequestInfo dictRequestInfo : dictionaryRequestInfoList)
            {
                if (dictRequestInfo.isInUse == false)
                {
                    if(dictionaryRequest.copy(dictRequestInfo.dictionaryRequest) == CodecReturnCodes.FAILURE)
                        return null;
                    
                    dictRequestInfo.channel = chnl;
                    dictRequestInfo.isInUse = true;
                    dictionaryRequestInfo = dictRequestInfo;
                    break;
                }
            }
        }

        return dictionaryRequestInfo;
    }

    @Override
    public Iterator<DictionaryRequestInfo> iterator()
    {
        return dictionaryRequestInfoList.iterator();
    }
}
