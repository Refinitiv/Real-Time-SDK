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
using static Refinitiv.Eta.Rdm.Directory;
using DataTypes = Refinitiv.Eta.Codec.DataTypes;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Directory Refresh. 
    /// Used by a Provider application to provide information about available services.
    /// </summary>
    public class DirectoryRefresh : MsgBase
    {
        private IRefreshMsg m_RefreshMsg = new Msg();
        private List<Service> m_ServiceList = new List<Service>(); 

        private Map m_Map = new Map();
        private MapEntry m_Entry = new MapEntry();
        private UInt tmpUInt = new UInt();
        private long m_SeqNum = 0;

        public override int StreamId { get => m_RefreshMsg.StreamId; set { m_RefreshMsg.StreamId = value; } }
        public override int MsgClass { get => m_RefreshMsg.MsgClass; }
        public override int DomainType { get => m_RefreshMsg.DomainType; }

        public DirectoryRefreshFlags Flags { get; set; }

        /// <summary>
        /// Checks the presence of service id field.
        /// </summary>
        public bool HasServiceId 
        { 
            get => (Flags & DirectoryRefreshFlags.HAS_SERVICE_ID) != 0; 
            set 
            { 
                if (value) 
                { Flags |= DirectoryRefreshFlags.HAS_SERVICE_ID; 
                } 
                else 
                    Flags &= ~DirectoryRefreshFlags.HAS_SERVICE_ID; 
            } 
        }
        /// <summary>
        /// Checks the presence of the Solicited flag. 
        /// </summary>
        public bool Solicited 
        { 
            get => (Flags & DirectoryRefreshFlags.SOLICITED) != 0; 
            set 
            { 
                if (value) 
                { Flags |= DirectoryRefreshFlags.SOLICITED;
                    m_RefreshMsg.ApplySolicited(); 
                }  
                else 
                    Flags &= ~DirectoryRefreshFlags.SOLICITED; 
            } 
        }
        /// <summary>
        /// Checks the presence of sequence number field.
        /// </summary>
        public bool HasSequenceNumber { get => (Flags & DirectoryRefreshFlags.HAS_SEQ_NUM) != 0; set { if (value) { Flags |= DirectoryRefreshFlags.HAS_SEQ_NUM; } else Flags &= ~DirectoryRefreshFlags.HAS_SEQ_NUM; } }
        /// <summary>
        /// Checks the presence of clear cache flag. 
        /// </summary>
        public bool ClearCache { get => (Flags & DirectoryRefreshFlags.CLEAR_CACHE) != 0; set { if (value) { Flags |= DirectoryRefreshFlags.CLEAR_CACHE; } else Flags &= ~DirectoryRefreshFlags.CLEAR_CACHE; } }
        /// <summary>
        /// Sequence number of this message.
        /// </summary>
        public long SequenceNumber 
        { 
            get => m_SeqNum; 
            set 
            { 
                Debug.Assert(HasSequenceNumber); 
                m_SeqNum = value; 
            } 
        }
        /// <summary>
        /// The current state of the stream.
        /// </summary>
        public State State { 
            get => m_RefreshMsg.State; 
            set 
            {
                Debug.Assert(value != null);
                m_RefreshMsg.State.StreamState(value.StreamState());
                m_RefreshMsg.State.DataState(value.DataState());
                m_RefreshMsg.State.Code(value.Code());
                m_RefreshMsg.State.Text(value.Text());
            } 
        }
        /// <summary>
        /// Filter indicating which filters may appear on this stream. 
        /// Where possible, this should match the consumer's request.
        /// </summary>
        public long Filter { get => m_RefreshMsg.MsgKey.Filter; set { m_RefreshMsg.MsgKey.Filter = value;  } }
        /// <summary>
        /// List of service entries.
        /// </summary>
        public List<Service> ServiceList
        {
            get => m_ServiceList;
            set
            {
                Debug.Assert(value != null);
                m_ServiceList.Clear();
                m_ServiceList.AddRange(value);
            }
        }
        /// <summary>
        /// The ID of the service whose information is provided by this stream 
        /// (if not present, all services should be provided). Should match 
        /// the Consumer's request if possible.
        /// </summary>
        public int ServiceId 
        { 
            get => m_RefreshMsg.MsgKey.ServiceId; 
            set { Debug.Assert(HasServiceId); m_RefreshMsg.MsgKey.ServiceId = value; } 
        }

        private Service? GetService(int serviceId)
        {
            foreach (Service service in m_ServiceList)
            {
                if (service.ServiceId == serviceId)
                    return service;
            }
            return null;
        }

        public override void Clear()
        {
            Flags = 0;
            m_RefreshMsg.Clear();
            m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
            m_RefreshMsg.DomainType = (int)Eta.Rdm.DomainType.SOURCE;
            m_RefreshMsg.ContainerType = DataTypes.MAP;
            m_RefreshMsg.ApplyHasMsgKey();
            m_RefreshMsg.MsgKey.ApplyHasFilter();
            m_RefreshMsg.ApplyRefreshComplete();
            m_SeqNum = 0;
            m_ServiceList.Clear();         
        }

        public DirectoryRefresh()
        {
            Clear();
        }

        public override CodecReturnCode Decode(DecodeIterator decIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.REFRESH)
            {
                return CodecReturnCode.FAILURE;
            }
            StreamId = msg.StreamId;

            IMsgKey msgKey = msg.MsgKey;
            if (msgKey != null)
            {
                if (msgKey.CheckHasFilter())
                {
                    Filter = msg.MsgKey.Filter;
                }
                if (msgKey.CheckHasServiceId())
                {
                    HasServiceId = true;
                    ServiceId = msgKey.ServiceId;
                }
            }
            IRefreshMsg refreshMsg = (IRefreshMsg)msg;
            if (refreshMsg.CheckSolicited())
                Solicited = true;
            if (refreshMsg.CheckClearCache())
                ClearCache = true;

            State = refreshMsg.State;
            
            if (refreshMsg.CheckHasSeqNum())
            {
                HasSequenceNumber = true;
                SequenceNumber = refreshMsg.SeqNum;
            }

            if (msg.ContainerType != DataTypes.MAP)
            {
                return CodecReturnCode.FAILURE;
            }

            return DecodeServiceList(decIter);
        }

        private CodecReturnCode DecodeServiceList(DecodeIterator dIter)
        {
            CodecReturnCode ret;
            if ((ret = m_Map.Decode(dIter)) < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            if (m_Map.ContainerType != DataTypes.FILTER_LIST || m_Map.KeyPrimitiveType != DataTypes.UINT)
            {
                return CodecReturnCode.FAILURE;
            }

            Service? service;
            while ((ret = m_Entry.Decode(dIter, tmpUInt)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS && ret != CodecReturnCode.BLANK_DATA)
                {
                    return ret;
                }

                service = GetService((int)tmpUInt.ToLong());
                if (service == null)
                {
                    service = new Service();
                    service.ServiceId = (int)tmpUInt.ToLong();
                    m_ServiceList.Add(service);
                }
                if (m_Entry.Action != MapEntryActions.DELETE)
                {
                    ret = service.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
                service.Action = m_Entry.Action;
            }

            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            if (ClearCache)
            {
                m_RefreshMsg.ApplyClearCache();
            }
            if (HasSequenceNumber)
            {
                m_RefreshMsg.ApplyHasSeqNum();
                m_RefreshMsg.SeqNum = m_SeqNum;
            }
            if (HasServiceId)
            {
                m_RefreshMsg.ApplyHasMsgKey();
                m_RefreshMsg.MsgKey.ApplyHasServiceId();
            }
            if (Solicited)
            {
                m_RefreshMsg.ApplySolicited();
            }
            CodecReturnCode ret = m_RefreshMsg.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.ENCODE_CONTAINER)
                return ret;
            ret = EncodeServiceList(encIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            ret = m_RefreshMsg.EncodeComplete(encIter, true);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode EncodeServiceList(EncodeIterator encIter)
        {
            m_Map.Clear();
            m_Map.Flags = MapFlags.NONE;
            m_Map.KeyPrimitiveType = DataTypes.UINT;
            m_Map.ContainerType = DataTypes.FILTER_LIST;
            CodecReturnCode ret = m_Map.EncodeInit(encIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            foreach (Service service in m_ServiceList)
            {
                m_Entry.Clear();
                m_Entry.Flags = MapEntryFlags.NONE;
                m_Entry.Action = service.Action;
                tmpUInt.Value(service.ServiceId);
                ret = m_Entry.EncodeInit(encIter, tmpUInt, 0);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                if (m_Entry.Action != MapEntryActions.DELETE)
                {
                    ret = service.Encode(encIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
                ret = m_Entry.EncodeComplete(encIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

            }
            return m_Map.EncodeComplete(encIter, true);
        }

        public CodecReturnCode Copy(DirectoryRefresh destRefreshMsg)
        {
            Debug.Assert(destRefreshMsg != null);
            destRefreshMsg.Clear();

            destRefreshMsg.StreamId = StreamId;
            destRefreshMsg.Filter = Filter;
            destRefreshMsg.State = State;

            if (ClearCache)
            {
                destRefreshMsg.ClearCache = true;
            }            
            if (Solicited)
            {
                destRefreshMsg.Solicited = true;
            }              
            if (HasServiceId)
            {
                destRefreshMsg.HasServiceId = true;
                destRefreshMsg.ServiceId = ServiceId;
            }
            if (HasSequenceNumber)
            {
                destRefreshMsg.HasSequenceNumber = true;
                destRefreshMsg.SequenceNumber = SequenceNumber;
            }

            CodecReturnCode ret;
            foreach (Service rdmService in ServiceList)
            {
                Service destRDMService = new Service();
                ret = rdmService.Copy(destRDMService);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                destRefreshMsg.ServiceList.Add(destRDMService);
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "DirectoryRefresh: \n");

            stringBuf.Append(tab);
            stringBuf.Append(State);
            stringBuf.Append(eol);

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.Append(eol);
            }

            if (HasSequenceNumber)
            {
                stringBuf.Append(tab);
                stringBuf.Append("sequenceNumber: ");
                stringBuf.Append(SequenceNumber);
                stringBuf.Append(eol);
            }

            stringBuf.Append(tab);
            stringBuf.Append("clearCache: ");
            stringBuf.Append(ClearCache);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("solicited: ");
            stringBuf.Append(Solicited);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("filter: ");
            bool addOr = false;
            if ((Filter & ServiceFilterFlags.INFO) != 0)
            {
                stringBuf.Append("INFO");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.DATA) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("DATA");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.GROUP) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("GROUP");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.LINK) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("LINK");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.LOAD) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("LOAD");
                addOr = true;
            }
            if ((Filter & ServiceFilterFlags.STATE) != 0)
            {
                if (addOr)
                    stringBuf.Append(" | ");
                stringBuf.Append("STATE");
                addOr = true;
            }
            stringBuf.Append(eol);

            foreach (Service service in ServiceList)
            {
                stringBuf.Append(service.ToString());
            }

            return stringBuf.ToString();
        }
    }
}
