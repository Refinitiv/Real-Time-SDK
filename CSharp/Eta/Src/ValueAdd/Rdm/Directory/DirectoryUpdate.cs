/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static Refinitiv.Eta.Rdm.Directory;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Directory Update. 
    /// Used by a Provider application to provide updates about available services. 
    /// </summary>
    public class DirectoryUpdate : MsgBase
    {
        private IUpdateMsg m_UpdateMsg = new Msg();
        private List<Service> m_ServiceList = new List<Service>();

        private Map m_Map = new Map();
        private MapEntry m_Entry = new MapEntry();
        private UInt m_tmpUInt = new UInt();
        private long m_SeqNum = 0;

        public DirectoryUpdateFlags Flags { get; set; }

        /// <summary>
        /// Checks the presence of the filter field.
        /// </summary>
        public bool HasFilter
        {
            get => (Flags & DirectoryUpdateFlags.HAS_FILTER) != 0;
            set
            {
                if (value)
                {
                    Flags |= DirectoryUpdateFlags.HAS_FILTER;
                }
                else
                {
                    Flags &= ~DirectoryUpdateFlags.HAS_FILTER;
                }
            }
        }
        /// <summary>
        /// Filter indicating which filters may appear on this stream. 
        /// Where possible, this should match the consumer's request. 
        /// Populated by <see cref="Directory.ServiceFilterFlags"/>
        /// </summary>
        public long Filter { get; set; }
        /// <summary>
        /// Checks the presence of service id field.
        /// </summary>
        public bool HasServiceId
        {
            get => (Flags & DirectoryUpdateFlags.HAS_SERVICE_ID) != 0;
            set
            {
                if (value)
                {
                    Flags |= DirectoryUpdateFlags.HAS_SERVICE_ID;
                }
                else
                {
                    Flags &= ~DirectoryUpdateFlags.HAS_SERVICE_ID;
                }
            }
        }
        /// <summary>
        /// The ID of the service whose information is provided by this stream 
        /// (if not present, all services should be provided). 
        /// Should match the Consumer's request if possible.
        /// </summary>
        public int ServiceId { get; set; }
        
        public override int StreamId { get => m_UpdateMsg.StreamId; set { m_UpdateMsg.StreamId = value; } }
        public override int DomainType { get => m_UpdateMsg.DomainType; }
        public override int MsgClass { get => m_UpdateMsg.MsgClass; }

        /// <summary>
        /// Handles the presence of sequence number field.
        /// </summary>
        public bool HasSequenceNumber
        {
            get => (Flags & DirectoryUpdateFlags.HAS_SEQ_NUM) != 0;
            set
            {
                if (value)
                {
                    Flags |= DirectoryUpdateFlags.HAS_SEQ_NUM;
                }
                else
                {
                    Flags &= ~DirectoryUpdateFlags.HAS_SEQ_NUM;
                }
            }
        }
        /// <summary>
        /// Sequence number of this message.
        /// </summary>
        public long SequenceNumber { get => m_SeqNum; set { m_SeqNum = value; } }
        /// <summary>
        /// List of service entries.
        /// </summary>
        public List<Service> ServiceList { 
            get => m_ServiceList; 
            set
            {
                Debug.Assert(value != null);
                m_ServiceList.Clear();
                m_ServiceList.AddRange(value);
            } 
        }

        private Service? GetService(int serviceId)
        {
            foreach (Service service in ServiceList)
            {
                if (service.ServiceId == serviceId)
                {
                    return service;
                }
            }

            return null;
        }

        public DirectoryUpdate()
        {
            Clear();
        }

        public override void Clear()
        {
            Flags = 0;
            m_UpdateMsg.Clear();
            m_UpdateMsg.MsgClass = MsgClasses.UPDATE;
            m_UpdateMsg.DomainType = (int)Eta.Rdm.DomainType.SOURCE;
            m_UpdateMsg.ContainerType = Codec.DataTypes.MAP;
            m_UpdateMsg.ApplyDoNotConflate();
            m_ServiceList.Clear();
            m_SeqNum = 0;
        }

        public override CodecReturnCode Decode(DecodeIterator decIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.UPDATE)
            {
                return CodecReturnCode.FAILURE;
            }
            StreamId = msg.StreamId;

            IMsgKey msgKey = msg.MsgKey;
            if (msgKey != null)
            {
                if (msgKey.CheckHasFilter())
                {
                    HasFilter = true;
                    Filter = msg.MsgKey.Filter;
                }
                if (msgKey.CheckHasServiceId())
                {
                    HasServiceId = true;
                    ServiceId = msgKey.ServiceId;
                }
            }
            IUpdateMsg updateMsg = (IUpdateMsg)msg;
            if (updateMsg.CheckHasSeqNum())
            {
                HasSequenceNumber = true;
                SequenceNumber = updateMsg.SeqNum;
            }

            if (msg.ContainerType != Codec.DataTypes.MAP)
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

            if (m_Map.ContainerType != Codec.DataTypes.FILTER_LIST || m_Map.KeyPrimitiveType != Codec.DataTypes.UINT)
            {
                return CodecReturnCode.FAILURE;
            }

            Service? service;
            while ((ret = m_Entry.Decode(dIter, m_tmpUInt)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS && ret != CodecReturnCode.BLANK_DATA)
                {
                    return ret;
                }

                service = GetService((int)m_tmpUInt.ToLong());
                if (service == null)
                {
                    service = new Service();
                    service.ServiceId = (int)m_tmpUInt.ToLong();
                    ServiceList.Add(service);
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

        private CodecReturnCode EncodeServiceList(EncodeIterator encIter)
        {
            m_Map.Clear();
            m_Map.Flags = MapFlags.NONE;
            m_Map.KeyPrimitiveType = Codec.DataTypes.UINT;
            m_Map.ContainerType = Codec.DataTypes.FILTER_LIST;
            CodecReturnCode ret = m_Map.EncodeInit(encIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            foreach (Service service in ServiceList)
            {
                m_Entry.Clear();
                m_Entry.Flags = MapEntryFlags.NONE;
                m_Entry.Action = service.Action;
                m_tmpUInt.Value(service.ServiceId);
                ret = m_Entry.EncodeInit(encIter, m_tmpUInt, 0);
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

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            if (HasFilter)
            {
                m_UpdateMsg.ApplyHasMsgKey();
                m_UpdateMsg.MsgKey.ApplyHasFilter();
                m_UpdateMsg.MsgKey.Filter = Filter;
            }

            if (HasServiceId)
            {
                m_UpdateMsg.ApplyHasMsgKey();
                m_UpdateMsg.MsgKey.ApplyHasServiceId();
                m_UpdateMsg.MsgKey.ServiceId = ServiceId;
            }

            if (HasSequenceNumber)
            {
                m_UpdateMsg.ApplyHasSeqNum();
                m_UpdateMsg.SeqNum = m_SeqNum;
            }

            CodecReturnCode ret = m_UpdateMsg.EncodeInit(encIter, 0);
            if (ret != CodecReturnCode.ENCODE_CONTAINER)
                return ret;
            ret = EncodeServiceList(encIter);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            ret = m_UpdateMsg.EncodeComplete(encIter, true);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            return CodecReturnCode.SUCCESS;
        }

        public CodecReturnCode Copy(DirectoryUpdate destUpdateMsg)
        {
            Debug.Assert(destUpdateMsg != null);
            destUpdateMsg.Clear();

            destUpdateMsg.StreamId = StreamId;

            if (HasFilter)
            {
                destUpdateMsg.HasFilter = true;
                destUpdateMsg.Filter = Filter;
            }

            if (HasServiceId)
            {
                destUpdateMsg.HasServiceId = true;
                destUpdateMsg.ServiceId = ServiceId;
            }


            if (HasSequenceNumber)
            {
                destUpdateMsg.HasSequenceNumber = true;
                destUpdateMsg.SequenceNumber = SequenceNumber;
            }

            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            foreach (Service rdmService in ServiceList)
            {
                Service destRDMService = new Service();
                ret = rdmService.Copy(destRDMService);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
                destUpdateMsg.ServiceList.Add(destRDMService);
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "DirectoryUpdate: \n");

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
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }
    }
}
