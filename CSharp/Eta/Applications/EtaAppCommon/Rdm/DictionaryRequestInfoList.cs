/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System.Collections;
using System.Collections.Generic;

namespace LSEG.Eta.Example.Common
{
    public class DictionaryRequestInfoList : IEnumerable<DictionaryRequestInfo>
    {
        private static readonly int MAX_DICTIONARY_REQUESTS_PER_CLIENT = 2;
        private static readonly int MAX_DICTIONARY_REQUESTS = (MAX_DICTIONARY_REQUESTS_PER_CLIENT * ProviderSession.NUM_CLIENT_SESSIONS);

        private List<DictionaryRequestInfo> m_DictionaryRequestInfoList = new List<DictionaryRequestInfo>(MAX_DICTIONARY_REQUESTS);

        public DictionaryRequestInfoList()
        {
            for (int i = 0; i < MAX_DICTIONARY_REQUESTS; i++)
            {
                m_DictionaryRequestInfoList.Add(new DictionaryRequestInfo());
            }
        }

        public void Init()
        {
            foreach (DictionaryRequestInfo dictionaryRequestInfo in m_DictionaryRequestInfoList)
            {
                dictionaryRequestInfo.Clear();
            }
        }

        public DictionaryRequestInfo? Get(IChannel chnl, DictionaryRequest dictionaryRequest)
        {
            DictionaryRequestInfo? dictionaryRequestInfo = null;

            /* first check if one already in use for this channel and stream id */
            foreach (DictionaryRequestInfo dictRequestInfo in m_DictionaryRequestInfoList)
            {
                if (dictRequestInfo.IsInUse &&
                        dictRequestInfo.Channel == chnl &&
                        dictRequestInfo.DictionaryRequest.StreamId == dictionaryRequest.StreamId)
                {
                    /*
                     * if dictionary name is different from last request, this is an
                     * invalid request
                     */
                    dictionaryRequestInfo = dictRequestInfo;
                    if (!dictionaryRequest.DictionaryName.Equals(dictRequestInfo.DictionaryRequest.DictionaryName))
                    {
                        return null;
                    }
                    break;
                }
            }

            /* get a new one if one is not already in use */
            if (dictionaryRequestInfo == null)
            {
                foreach (DictionaryRequestInfo dictRequestInfo in m_DictionaryRequestInfoList)
                {
                    if (dictRequestInfo.IsInUse == false)
                    {
                        if (dictionaryRequest.Copy(dictRequestInfo.DictionaryRequest) == CodecReturnCode.FAILURE)
                            return null;

                        dictRequestInfo.Channel = chnl;
                        dictRequestInfo.IsInUse = true;
                        dictionaryRequestInfo = dictRequestInfo;
                        break;
                    }
                }
            }

            return dictionaryRequestInfo;
        }

        public IEnumerator<DictionaryRequestInfo> GetEnumerator()
        {
            return m_DictionaryRequestInfoList.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
