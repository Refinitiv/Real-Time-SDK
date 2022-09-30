/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.ValueAdd.Rdm;
using System;
using System.Diagnostics;
using System.Text;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.Example.Common
{
    public class SymbolListRequest : MsgBase
    {
        protected Msg m_RequestMsg = new Msg();
        private Qos qos = new Qos();
        private Buffer symbolListName = new Buffer();
        public override int DomainType { get => m_RequestMsg.DomainType; }
        public override int StreamId { get => m_RequestMsg.StreamId; set { m_RequestMsg.StreamId = value; } }
        public override int MsgClass { get => m_RequestMsg.MsgClass; }
        
        public SymbolListRequestFlags Flags { get; set; }

        public bool HasServiceId
        {
            get => (Flags & SymbolListRequestFlags.HAS_SERVICE_ID) != 0;
            set
            {
                if (value)
                {
                    Flags |= SymbolListRequestFlags.HAS_SERVICE_ID;
                }
                else
                    Flags &= ~SymbolListRequestFlags.HAS_SERVICE_ID;
            }
        }
        public bool HasPriority
        {
            get => (Flags & SymbolListRequestFlags.HAS_PRIORITY) != 0;
            set
            {
                if (value)
                {
                    Flags |= SymbolListRequestFlags.HAS_PRIORITY;
                }
                else
                    Flags &= ~SymbolListRequestFlags.HAS_PRIORITY;
            }
        }
        public bool Streaming
        {
            get => (Flags & SymbolListRequestFlags.STREAMING) != 0;
            set
            {
                if (value) { Flags |= SymbolListRequestFlags.STREAMING; }
                else Flags &= ~SymbolListRequestFlags.STREAMING;
            }
        }
        public bool HasQos
        {
            get => (Flags & SymbolListRequestFlags.HAS_QOS) != 0;
            set
            {
                if (value)
                {
                    Flags |= SymbolListRequestFlags.HAS_QOS;
                }
                else
                    Flags &= ~SymbolListRequestFlags.HAS_QOS;
            }
        }
        public Qos Qos
        {
            get => qos;
            set
            {
                Debug.Assert(value != null);
                value.Copy(qos);
            }
        }
        public int ServiceId { get; set; }
        public Buffer SymbolListName { get => symbolListName; set { Debug.Assert(value != null); symbolListName.Data(value.Data(), value.Position, value.Length); } }
        
        public int PriorityClass { get; set; }
        public int PriorityCount { get; set; }

        public SymbolListRequest()
        {
            Clear();
        }

        public override void Clear()
        {
            m_RequestMsg.Clear();
            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            m_RequestMsg.DomainType = (int)Rdm.DomainType.SYMBOL_LIST;
            m_RequestMsg.ContainerType = DataTypes.NO_DATA;
            symbolListName.Clear();
            qos.Clear();
            Flags = 0;
        }

        public override CodecReturnCode Decode(DecodeIterator encIter, Msg msg)
        {
            throw new NotImplementedException();
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            if (Streaming)
            {
                m_RequestMsg.ApplyStreaming();
            }

            if (HasPriority)
            {
                m_RequestMsg.ApplyHasPriority();
                m_RequestMsg.Priority.PriorityClass = PriorityClass;
                m_RequestMsg.Priority.Count = PriorityCount;
            }

            if (HasQos)
            {
                m_RequestMsg.ApplyHasQos();
                qos.Copy(m_RequestMsg.Qos);
            }

            m_RequestMsg.MsgKey.ApplyHasNameType();
            m_RequestMsg.MsgKey.ApplyHasName();
            m_RequestMsg.MsgKey.ApplyHasServiceId();
            m_RequestMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            m_RequestMsg.MsgKey.Name.Data(SymbolListName.Data(), SymbolListName.Position, SymbolListName.Length);
            m_RequestMsg.MsgKey.ServiceId = ServiceId;


            return m_RequestMsg.Encode(encIter);
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "SymbolListRequest: \n");
            stringBuf.Append(tab);
            stringBuf.Append("streaming: ");
            stringBuf.Append(Streaming);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("symbolListName: ");
            stringBuf.Append(SymbolListName);
            stringBuf.Append(eol);

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.Append(eol);
            }
            if (HasPriority)
            {
                stringBuf.Append(tab);
                stringBuf.Append("priority class: ");
                stringBuf.Append(PriorityClass);
                stringBuf.Append(", priority count: ");
                stringBuf.Append(PriorityCount);
                stringBuf.Append(eol);
            }
            if (HasQos)
            {
                stringBuf.Append(tab);
                stringBuf.Append("qos: ");
                stringBuf.Append(qos.ToString());
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }
    }
}
