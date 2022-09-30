/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;
using System.Collections;
using System.Collections.Generic;

namespace Refinitiv.Eta.Example.Common
{
    public class DirectoryRequestInfoList : IEnumerable<DirectoryRequestInfo>
    {
        private List<DirectoryRequestInfo> m_DirectoryRequestInfoList = new List<DirectoryRequestInfo>(ProviderSession.NUM_CLIENT_SESSIONS);

        public DirectoryRequestInfoList()
        {
            for (int i = 0; i < ProviderSession.NUM_CLIENT_SESSIONS; i++)
            {
                m_DirectoryRequestInfoList.Add(new DirectoryRequestInfo());
            }
        }

        public void Init()
        {
            foreach (DirectoryRequestInfo srcDirRequestInfo in m_DirectoryRequestInfoList)
            {
                srcDirRequestInfo.Clear();
            }
        }

        public DirectoryRequest? Get(IChannel chnl, DirectoryRequest directoryRequest)
        {
            DirectoryRequest? storedRequest = null;

            /* first check if one already in use for this channel */
            foreach (DirectoryRequestInfo sourceDirectoryReqInfo in m_DirectoryRequestInfoList)
            {
                if (sourceDirectoryReqInfo.IsInUse &&
                        object.ReferenceEquals(sourceDirectoryReqInfo.Channel,chnl))
                {
                    storedRequest = sourceDirectoryReqInfo.DirectoryRequest;
                    /*
                     * if stream id is different from last request, this is an
                     * invalid request
                     */
                    if (storedRequest.StreamId != storedRequest.StreamId)
                    {
                        return null;
                    }
                    break;
                }
            }

            /* get a new one if one is not already in use */
            if (storedRequest is null)
            {
                foreach (DirectoryRequestInfo sourceDirectoryReqInfo in m_DirectoryRequestInfoList)
                {
                    if (sourceDirectoryReqInfo.IsInUse == false)
                    {
                        storedRequest = sourceDirectoryReqInfo.DirectoryRequest;
                        sourceDirectoryReqInfo.Channel = chnl;
                        sourceDirectoryReqInfo.IsInUse = true;
                        directoryRequest.Copy(storedRequest);
                        break;
                    }
                }
            }

            return storedRequest;
        }

        public IEnumerator<DirectoryRequestInfo> GetEnumerator()
        {
            return m_DirectoryRequestInfoList.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
