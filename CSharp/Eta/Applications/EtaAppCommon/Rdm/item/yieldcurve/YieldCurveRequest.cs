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
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Array = Refinitiv.Eta.Codec.Array;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.Example.Common
{
    public class YieldCurveRequest : MsgBase
    {
        private List<string> itemNames = new List<string>();
        private List<string> viewFieldList = new List<string>();

        private ElementList elementList = new ElementList();
        private ElementEntry elementEntry = new ElementEntry();
        private Array array = new Array();
        private ArrayEntry arrayEntry = new ArrayEntry();
        private Buffer itemNameBuf = new Buffer();

        protected Msg m_RequestMsg = new Msg();
        private Qos qos = new Qos();

        public override int DomainType { get => m_RequestMsg.DomainType; }
        public override int StreamId { get => m_RequestMsg.StreamId; set { m_RequestMsg.StreamId = value; } }
        public override int MsgClass { get => m_RequestMsg.MsgClass; }

        public YieldCurveRequestFlags Flags { get; set; }

        public bool HasServiceId
        {
            get => (Flags & YieldCurveRequestFlags.HAS_SERVICE_ID) != 0;
            set
            {
                if (value)
                {
                    Flags |= YieldCurveRequestFlags.HAS_SERVICE_ID;
                }
                else
                    Flags &= ~YieldCurveRequestFlags.HAS_SERVICE_ID;
            }
        }
        public bool HasPriority
        {
            get => (Flags & YieldCurveRequestFlags.HAS_PRIORITY) != 0;
            set
            {
                if (value)
                {
                    Flags |= YieldCurveRequestFlags.HAS_PRIORITY;
                }
                else
                    Flags &= ~YieldCurveRequestFlags.HAS_PRIORITY;
            }
        }
        public bool IsPrivateStream
        {
            get => (Flags & YieldCurveRequestFlags.PRIVATE_STREAM) != 0;
            set
            {
                if (value) { Flags |= YieldCurveRequestFlags.PRIVATE_STREAM; }
                else Flags &= ~YieldCurveRequestFlags.PRIVATE_STREAM;
            }
        }
        public bool HasQos
        {
            get => (Flags & YieldCurveRequestFlags.HAS_QOS) != 0;
            set
            {
                if (value)
                {
                    Flags |= YieldCurveRequestFlags.HAS_QOS;
                }
                else
                    Flags &= ~YieldCurveRequestFlags.HAS_QOS;
            }
        }
        
        public bool Streaming
        {
            get => (Flags & YieldCurveRequestFlags.STREAMING) != 0;
            set
            {
                if (value) { Flags |= YieldCurveRequestFlags.STREAMING; }
                else Flags &= ~YieldCurveRequestFlags.STREAMING;
            }
        }
        public bool HasView
        {
            get => (Flags & YieldCurveRequestFlags.HAS_VIEW) != 0;
            set
            {
                if (value) { Flags |= YieldCurveRequestFlags.HAS_VIEW; }
                else Flags &= ~YieldCurveRequestFlags.HAS_VIEW;
            }
        }
        public State State
        {
            get => m_RequestMsg.State;
            set
            {
                Debug.Assert(value != null);
                value.Copy(m_RequestMsg.State);
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
        public List<string> ItemNames
        {
            get => itemNames;
            set
            {
                Debug.Assert(value != null);
                itemNames.Clear();
                itemNames.AddRange(value);
            }
        }
        public int PriorityClass { get; set; }
        public int PriorityCount { get; set; }
        public List<string> ViewFields
        {
            get => viewFieldList;
            set
            {
                Debug.Assert(value != null);
                viewFieldList.Clear();
                viewFieldList.AddRange(value);
            }
        }

        public YieldCurveRequest()
        {
            Clear();
        }
        public override void Clear()
        {
            Flags = 0;
            qos.Clear();
            itemNames.Clear();
            PriorityClass = 1;
            PriorityCount = 1;
            viewFieldList.Clear();
            m_RequestMsg.Clear();
            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            m_RequestMsg.DomainType = (int)Rdm.DomainType.YIELD_CURVE;
            m_RequestMsg.ContainerType = DataTypes.NO_DATA;
        }

        public override CodecReturnCode Decode(DecodeIterator encIter, Msg msg)
        {
            throw new NotImplementedException();
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            elementList.Clear();
            elementEntry.Clear();
            array.Clear();
            itemNameBuf.Clear();

            if (HasQos)
            {
                m_RequestMsg.ApplyHasQos();
                qos.Copy(m_RequestMsg.Qos);
            }

            if (HasPriority)
            {
                m_RequestMsg.ApplyHasPriority();
                m_RequestMsg.Priority.PriorityClass = PriorityClass;
                m_RequestMsg.Priority.Count = PriorityCount;
            }

            if (Streaming)
            {
                m_RequestMsg.ApplyStreaming();
            }

            bool isBatchRequest = itemNames.Count > 1;
            ApplyFeatureFlags(isBatchRequest);

            m_RequestMsg.MsgKey.ApplyHasServiceId();
            m_RequestMsg.MsgKey.ServiceId = ServiceId;

            m_RequestMsg.MsgKey.ApplyHasNameType();
            m_RequestMsg.MsgKey.NameType = InstrumentNameTypes.RIC;

            if (!isBatchRequest)
            {
                m_RequestMsg.MsgKey.ApplyHasName();
                m_RequestMsg.MsgKey.Name.Data(ItemNames[0]);
            }

            CodecReturnCode ret = m_RequestMsg.EncodeInit(encIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            /* encode request message payload */
            if (HasView || isBatchRequest)
            {
                ret = EncodeRequestPayload(isBatchRequest, encIter);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
            }
            ret = m_RequestMsg.EncodeComplete(encIter, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode EncodeRequestPayload(bool isBatchRequest, EncodeIterator encodeIter)
        {
            m_RequestMsg.ContainerType = DataTypes.ELEMENT_LIST;
            elementList.ApplyHasStandardData();

            CodecReturnCode ret = elementList.EncodeInit(encodeIter, null, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            if (isBatchRequest && (EncodeBatchRequest(encodeIter) < CodecReturnCode.SUCCESS))
            {
                return CodecReturnCode.FAILURE;
            }

            ret = elementList.EncodeComplete(encodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        private void ApplyFeatureFlags(bool isBatchRequest)
        {
            if (IsPrivateStream)
            {
                m_RequestMsg.ApplyPrivateStream();
            }

            if (HasView || isBatchRequest)
            {
                m_RequestMsg.ContainerType = DataTypes.ELEMENT_LIST;
                if (HasView)
                {
                    m_RequestMsg.ApplyHasView();
                }
                if (isBatchRequest)
                {
                    m_RequestMsg.ApplyHasBatch();
                }
            }
        }

        private CodecReturnCode EncodeBatchRequest(EncodeIterator encodeIter)
        {
            /*
             * For Batch requests, the message has a payload of an element list that
             * contains an array of the requested items
             */
            elementEntry.Name = ElementNames.BATCH_ITEM_LIST;
            elementEntry.DataType = DataTypes.ARRAY;
            CodecReturnCode ret = elementEntry.EncodeInit(encodeIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            /* Encode the array of requested item names */
            array.PrimitiveType = DataTypes.ASCII_STRING;
            array.ItemLength = 0;

            ret = array.EncodeInit(encodeIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            foreach (string itemName in ItemNames)
            {
                arrayEntry.Clear();
                itemNameBuf.Data(itemName);
                ret = arrayEntry.Encode(encodeIter, itemNameBuf);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            ret = array.EncodeComplete(encodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            ret = elementEntry.EncodeComplete(encodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "YieldCurveRequest: \n");
            stringBuf.Append(tab);
            stringBuf.Append("streaming: ");
            stringBuf.Append(Streaming);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("domain: ");
            stringBuf.Append(DomainTypes.ToString(m_RequestMsg.DomainType));
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("isPrivateStream: ");
            stringBuf.Append(IsPrivateStream);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("hasView: ");
            stringBuf.Append(HasView);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("itemNames: ");
            foreach (var name in ItemNames)
            {
                stringBuf.Append(name);
                stringBuf.Append(" ");
            }
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
