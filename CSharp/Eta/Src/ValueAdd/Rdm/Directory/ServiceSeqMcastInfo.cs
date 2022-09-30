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
    /// The RDM Service Sequenced Multicast  Info. 
    /// Contains information provided by the Source Directory Link filter.
    /// </summary>
    public class ServiceSeqMcastInfo
    {
        internal const int RDM_SVC_SMF_HAS_SNAPSHOT_SERV = 0x001;   // (0x001) Indicates presence of Snapshot Server Info
        internal const int RDM_SVC_SMF_HAS_GAP_REC_SERV = 0x002;   // (0x002) Indicates presence of Gap Recovery Server Info
        internal const int RDM_SVC_SMF_HAS_REF_DATA_SERV = 0x004;  // (0x004) Indicates presence of Reference Data Server Info
        internal const int RDM_SVC_SMF_HAS_SMC_SERV = 0x008;   // (0x008) Indicates presence of Streaming Multicast Channels Server Info
        internal const int RDM_SVC_SMF_HAS_GMC_SERV = 0x010;    // (0x010) Indicates presence of Gap Multicast Channel Server Info

        private const string eol = "\n";
        private const string tab = "\t";

        private StringBuilder m_StringBuf = new StringBuilder();

        private Buffer m_MapKey = new Buffer();
        private Map m_LinkMap = new Map();
        private MapEntry m_LinkMapEntry = new MapEntry();
                                                                                         
        private FilterEntryActions m_Actions;
        private List<ServiceLink> m_LinkList;

        public RDMAddressPortInfo SnapshotServer { get; set; } = new RDMAddressPortInfo();     
        public RDMAddressPortInfo GapRecoveryServer { get; set; } = new RDMAddressPortInfo();                                  
        public RDMAddressPortInfo RefDataServer { get; set; } = new RDMAddressPortInfo();                  
        public int StreamingMCastChanServerCount { get; set; }                                         
        public RDMMCAddressPortInfo StreamingMcastChanServerList { get; set; } = new RDMMCAddressPortInfo();       
        public int GapMCastChanServerCount { get; set; }                                                     
        public RDMMCAddressPortInfo GapMCastChanServerList { get; set; } = new RDMMCAddressPortInfo();                 

        /// <summary>
        /// The action associated with this link filter.
        /// </summary>
        public FilterEntryActions Action { get; set; }
        /// <summary>
        /// The list of entries with information about upstream sources.
        /// </summary>
        public List<ServiceLink> LinkList { 
            get => m_LinkList; 
            set 
            {
                Debug.Assert(value != null);
                m_LinkList.Clear();
                m_LinkList.AddRange(value);
            } 
        }
        public int Flags { get; set; }

        public ServiceSeqMcastInfo()
        {
            m_LinkList = new List<ServiceLink>();    
        }

        public CodecReturnCode Encode(EncodeIterator encIter)
        {
            m_LinkMap.Clear();
            m_LinkMap.Flags = MapFlags.NONE;
            m_LinkMap.ContainerType = DataTypes.ELEMENT_LIST;
            m_LinkMap.KeyPrimitiveType = DataTypes.ASCII_STRING;

            CodecReturnCode ret = m_LinkMap.EncodeInit(encIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            foreach (ServiceLink serviceLink in LinkList)
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

        public CodecReturnCode Decode(DecodeIterator dIter)
        {
            CodecReturnCode ret;
            ServiceSeqMcast seqMcast = new ServiceSeqMcast();
            seqMcast.Name.Data(m_MapKey.Data(), m_MapKey.Position, m_MapKey.Length);
            seqMcast.Action = m_LinkMapEntry.Action;
            if (seqMcast.Action != MapEntryActions.DELETE)
            {
                ret = seqMcast.Decode(dIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            Flags = seqMcast.SeqMcastInfo.Flags;
            m_Actions = seqMcast.SeqMcastInfo.m_Actions;
            SnapshotServer = seqMcast.SeqMcastInfo.SnapshotServer;
            GapRecoveryServer = seqMcast.SeqMcastInfo.GapRecoveryServer;
            RefDataServer = seqMcast.SeqMcastInfo.RefDataServer;
            StreamingMCastChanServerCount = seqMcast.SeqMcastInfo.StreamingMCastChanServerCount;
            StreamingMcastChanServerList = seqMcast.SeqMcastInfo.StreamingMcastChanServerList;
            GapMCastChanServerCount = seqMcast.SeqMcastInfo.GapMCastChanServerCount;
            GapMCastChanServerList = seqMcast.SeqMcastInfo.GapMCastChanServerList;

            return CodecReturnCode.SUCCESS;
        }

        public void Clear()
        {
            LinkList.Clear();
            Flags = 0;
            SnapshotServer.Clear();
            GapRecoveryServer.Clear();
            RefDataServer.Clear();
            StreamingMcastChanServerList.Clear();
            StreamingMCastChanServerCount = 0;
            GapMCastChanServerCount = 0;
            GapMCastChanServerList.Clear();
            Action = FilterEntryActions.SET;
        }

        public override string ToString()
        {
            foreach (ServiceLink link in LinkList)
            {
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append("LinkFilter: ");
                m_StringBuf.Append(eol);
                m_StringBuf.Append(link);
            }

            return m_StringBuf.ToString();
        }

        public CodecReturnCode Copy(ServiceLinkInfo destServiceLinkInfo)
        {
            Debug.Assert(destServiceLinkInfo != null);
            destServiceLinkInfo.Clear();
            destServiceLinkInfo.Action = Action;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            foreach (ServiceLink serviceLink in LinkList)
            {
                ServiceLink serviceLink2 = new ServiceLink();
                ret = serviceLink.Copy(serviceLink2);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                destServiceLinkInfo.LinkList.Add(serviceLink2);
            }

            return ret;
        }

        public Buffer SnapshotServerAddress()
        {
            return SnapshotServer.Address;
        }

        public long SnapshotServerPort()
        {
            return SnapshotServer.Port;
        }

        public Buffer RefDataServerAddress()
        {
            return RefDataServer.Address;
        }

        public long RefDataServerPort()
        {
            return RefDataServer.Port;
        }

        public Buffer GapRecoveryServerAddress()
        {
            return GapRecoveryServer.Address;
        }

        public long GapRecoveryServerPort()
        {
            return GapRecoveryServer.Port;
        }

        public List<Buffer> GapMCastChanServerListAddr()
        {
            return GapMCastChanServerList.Address;
        }

        public List<long> GapMCastChanPortList()
        {
            return GapMCastChanServerList.Port;
        }

        public List<long> GapMCastChanDomainList()
        {
            return GapMCastChanServerList.Domain;
        }

        public List<Buffer> StreamingMCastChanServerAddrList()
        {
            return StreamingMcastChanServerList.Address;
        }

        public List<long> StreamingMCastChanPortList()
        {
            return StreamingMcastChanServerList.Port;
        }

        public List<long> StreamingMCastChanDomainList()
        {
            return StreamingMcastChanServerList.Domain;
        }
    }
}
