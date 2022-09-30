/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using System.Diagnostics;
using System.Text;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service Link Info. 
    /// Contains information provided by the Source Directory Link filter.
    /// </summary>
    public class ServiceLinkInfo
    {

        private const string eol = "\n";
        private const string tab = "\t";

        private StringBuilder m_StringBuf = new StringBuilder();

        private Buffer m_MapKey = new Buffer();
        private Map m_LinkMap = new Map();
        private MapEntry m_LinkMapEntry = new MapEntry();

        private List<ServiceLink> m_LinkList;

        /// <summary>
        /// List of entries with information about upstream sources.
        /// </summary>
        public List<ServiceLink> LinkList { 
            get => m_LinkList; 
            set 
            { 
                Debug.Assert(value != null); 
                m_LinkList.Clear();
                foreach (ServiceLink serviceLink in value)
                {
                    m_LinkList.Add(serviceLink);
                }
            } 
        }

        /// <summary>
        /// The action associated with this link filter.
        /// </summary>
        public FilterEntryActions Action { get; set; }

        /// <summary>
        /// Instantiates a new service link info.
        /// </summary>
        public ServiceLinkInfo()
        {
            m_LinkList = new List<ServiceLink>();
        }

        /// <summary>
        /// Encode an RDM Service Link filter.
        /// </summary>
        /// <param name="encIter"><see cref="EncodeIterator"/> instance.</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation.</returns>
        public CodecReturnCode Encode(EncodeIterator encIter)
        {
            m_LinkMap.Clear();
            m_LinkMap.Flags = MapFlags.NONE;
            m_LinkMap.ContainerType = DataTypes.ELEMENT_LIST;
            m_LinkMap.KeyPrimitiveType = DataTypes.ASCII_STRING;

            CodecReturnCode ret = m_LinkMap.EncodeInit(encIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            foreach (ServiceLink serviceLink in m_LinkList)
            {
                m_LinkMapEntry.Clear();
                m_LinkMapEntry.Flags = MapEntryFlags.NONE;
                m_LinkMapEntry.Action = serviceLink.Action;
                if (m_LinkMapEntry.Action == MapEntryActions.DELETE)
                {
                    ret = m_LinkMapEntry.Encode(encIter, serviceLink.Name);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = m_LinkMapEntry.EncodeInit(encIter, serviceLink.Name, 0);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;

                    ret = serviceLink.Encode(encIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;

                    ret = m_LinkMapEntry.EncodeComplete(encIter, true);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;

                }
            }

            return m_LinkMap.EncodeComplete(encIter, true);
        }

        /// <summary>
        /// Decode a ETA service link filter into an RDM service link filter.
        /// </summary>
        /// <param name="dIter"><see cref="DecodeIterator"/> instance.</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation.</returns>
        public CodecReturnCode Decode(DecodeIterator dIter)
        {
            Clear();
            m_LinkMap.Clear();
            m_LinkMapEntry.Clear();
            m_MapKey.Clear();

            CodecReturnCode ret = m_LinkMap.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if (m_LinkMap.ContainerType != DataTypes.ELEMENT_LIST || (m_LinkMap.KeyPrimitiveType != DataTypes.BUFFER && m_LinkMap.KeyPrimitiveType != DataTypes.ASCII_STRING))
                return CodecReturnCode.FAILURE;

            while ((ret = m_LinkMapEntry.Decode(dIter, m_MapKey)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS && ret != CodecReturnCode.BLANK_DATA)
                    return ret;

                ServiceLink serviceLink = new ServiceLink();
                serviceLink.Name.Data(m_MapKey.Data(), m_MapKey.Position, m_MapKey.Length);
                serviceLink.Action = m_LinkMapEntry.Action;
                if (serviceLink.Action != MapEntryActions.DELETE)
                {
                    ret = serviceLink.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    m_LinkList.Add(serviceLink);
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Clears current ServiceLinkInfo instance.
        /// </summary>
        public void Clear()
        {
            m_LinkList.Clear();
        }

        public override string ToString()
        {
            foreach (ServiceLink link in m_LinkList)
            {
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append("LinkFilter: ");
                m_StringBuf.Append(eol);
                m_StringBuf.Append(link.ToString());
            }

            return m_StringBuf.ToString();
        }

        public CodecReturnCode Copy(ServiceLinkInfo destServiceLinkInfo)
        {
            Debug.Assert(destServiceLinkInfo != null);
            destServiceLinkInfo.Clear();
            destServiceLinkInfo.Action = Action;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            foreach (ServiceLink serviceLink in m_LinkList)
            {
                ServiceLink serviceLink2 = new ServiceLink();
                ret = serviceLink.Copy(serviceLink2);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                destServiceLinkInfo.LinkList.Add(serviceLink2);
            }

            return ret;
        }

        /// <summary>
        /// Performs an update of ServiceLinkInfo object.
        /// </summary>
        /// <param name="destServiceLinkInfo">ServiceLinkInfo object to update with information from this object. It cannot be null.</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation.</returns>
        public CodecReturnCode Update(ServiceLinkInfo destServiceLinkInfo)
        {
            Debug.Assert(destServiceLinkInfo != null);

            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            destServiceLinkInfo.Action = Action;
            for (int i = 0; i < m_LinkList.Count; ++i)
            {
                switch (m_LinkList[i].Action)
                {
                    case MapEntryActions.ADD:
                    case MapEntryActions.UPDATE:
                        bool foundService = false;
                        for (int j = 0; j < destServiceLinkInfo.LinkList.Count; ++j)
                        {
                            if (destServiceLinkInfo.LinkList[j].Name.Equals(LinkList[i].Name))
                            {
                                ret = m_LinkList[i].Copy(destServiceLinkInfo.LinkList[j]);
                                foundService = true;
                                break;
                            }
                        }
                        if (ret != CodecReturnCode.SUCCESS)
                            return ret;

                        if (!foundService)
                        {
                            ServiceLink serviceLink = new ServiceLink();
                            destServiceLinkInfo.LinkList.Add(serviceLink);
                            ret = m_LinkList[i].Copy(serviceLink);
                            if (ret != CodecReturnCode.SUCCESS)
                                return ret;
                        }
                        break;
                    case MapEntryActions.DELETE:
                        for (int j = 0; j < destServiceLinkInfo.LinkList.Count; ++j)
                        {
                            if (destServiceLinkInfo.LinkList[j].Name.Equals(m_LinkList[i].Name))
                            {
                                var ll = destServiceLinkInfo.LinkList[j];
                                destServiceLinkInfo.LinkList.Remove(ll);
                                break;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            return ret;
        }
    }
}
