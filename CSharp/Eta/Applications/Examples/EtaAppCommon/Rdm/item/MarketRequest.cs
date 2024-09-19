/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Array = LSEG.Eta.Codec.Array;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Base class for Market domain request messages.
    /// </summary>
    public class MarketRequest : MsgBase
    {
        private List<string> itemNames = new List<string>();
        private List<string> viewFieldList = new List<string>();

        private const int VIEW_TYPE = ViewTypes.FIELD_ID_LIST;

        private ElementList elementList = new ElementList();
        private ElementEntry elementEntry = new ElementEntry();
        private Array array = new Array();
        private ArrayEntry arrayEntry = new ArrayEntry();
        private Buffer itemNameBuf = new Buffer();

        private Int tempInt = new Int();
        private UInt tempUInt = new UInt();
        private Array viewArray = new Array();

        protected Msg m_RequestMsg = new Msg();
        private Qos qos = new Qos();

        public override int DomainType { get => m_RequestMsg.DomainType; }
        public override int StreamId { get => m_RequestMsg.StreamId; set { m_RequestMsg.StreamId = value; } }
        public override int MsgClass { get => m_RequestMsg.MsgClass; }

        public MarketPriceRequestFlags Flags { get; set; }

        public bool HasServiceId
        {
            get => (Flags & MarketPriceRequestFlags.HAS_SERVICE_ID) != 0;
            set
            {
                if (value)
                {
                    Flags |= MarketPriceRequestFlags.HAS_SERVICE_ID;
                }
                else
                    Flags &= ~MarketPriceRequestFlags.HAS_SERVICE_ID;
            }
        }
        public bool HasPriority
        {
            get => (Flags & MarketPriceRequestFlags.HAS_PRIORITY) != 0;
            set
            {
                if (value)
                {
                    Flags |= MarketPriceRequestFlags.HAS_PRIORITY;
                }
                else
                    Flags &= ~MarketPriceRequestFlags.HAS_PRIORITY;
            }
        }
        public bool PrivateStream
        {
            get => (Flags & MarketPriceRequestFlags.PRIVATE_STREAM) != 0;
            set
            {
                if (value) { Flags |= MarketPriceRequestFlags.PRIVATE_STREAM; }
                else Flags &= ~MarketPriceRequestFlags.PRIVATE_STREAM;
            }
        }
        public bool HasQos
        {
            get => (Flags & MarketPriceRequestFlags.HAS_QOS) != 0;
            set
            {
                if (value)
                {
                    Flags |= MarketPriceRequestFlags.HAS_QOS;
                }
                else
                    Flags &= ~MarketPriceRequestFlags.HAS_QOS;
            }
        }
        public bool HasWorstQos
        {
            get => (Flags & MarketPriceRequestFlags.HAS_WORST_QOS) != 0;
            set
            {
                if (value)
                {
                    Flags |= MarketPriceRequestFlags.HAS_WORST_QOS;
                }
                else
                    Flags &= ~MarketPriceRequestFlags.HAS_WORST_QOS;
            }
        }
        public bool Streaming
        {
            get => (Flags & MarketPriceRequestFlags.STREAMING) != 0;
            set
            {
                if (value) { Flags |= MarketPriceRequestFlags.STREAMING; }
                else Flags &= ~MarketPriceRequestFlags.STREAMING;
            }
        }
        public bool HasView
        {
            get => (Flags & MarketPriceRequestFlags.HAS_VIEW) != 0;
            set
            {
                if (value) { Flags |= MarketPriceRequestFlags.HAS_VIEW; }
                else Flags &= ~MarketPriceRequestFlags.HAS_VIEW;
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
        public List<string> ItemNames { 
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
        public int Identifier { get; set; }
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
        
        public MarketRequest()
        {
            Clear();
        }

        public MarketRequest(int domainType)
        {
            Clear();
            qos = new Qos();
            itemNames = new List<string>();
            viewFieldList = new List<string>();
            PriorityClass = 1;
            PriorityCount = 1;
            Flags = 0;
            Identifier = -1;
            m_RequestMsg.DomainType = domainType;
        }
       
        public bool HasIdentifier()
        {
            if (Identifier >= 0)
                return true;
            else
                return false;
        }

        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            elementList.Clear();
            elementEntry.Clear();
            array.Clear();
            itemNameBuf.Clear();

            /* set-up message */
            
            if (HasQos)
            {
                m_RequestMsg.ApplyHasQos();
                Qos.Copy(m_RequestMsg.Qos);
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

            bool isBatchRequest = ItemNames.Count > 1;
            ApplyFeatureFlags(isBatchRequest);

            /* specify msgKey members */

            m_RequestMsg.MsgKey.ApplyHasServiceId();
            m_RequestMsg.MsgKey.ServiceId = ServiceId;

            m_RequestMsg.MsgKey.ApplyHasNameType();
            m_RequestMsg.MsgKey.NameType = InstrumentNameTypes.RIC;

            /* If user set Identifier */
            if (HasIdentifier())
            {
                m_RequestMsg.MsgKey.ApplyHasIdentifier();
                m_RequestMsg.MsgKey.Identifier = Identifier;
            }

            if (!isBatchRequest && !HasIdentifier())
            {
                m_RequestMsg.MsgKey.ApplyHasName();
                m_RequestMsg.MsgKey.Name.Data(ItemNames[0]);
            }

            CodecReturnCode ret = m_RequestMsg.EncodeInit(encodeIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            /* encode request message payload */
            if (HasView || isBatchRequest)
            {
                ret = EncodeRequestPayload(isBatchRequest, encodeIter);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
            }
            ret = m_RequestMsg.EncodeComplete(encodeIter, true);
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

            if (isBatchRequest
                    && (EncodeBatchRequest(encodeIter) < CodecReturnCode.SUCCESS))
            {
                return CodecReturnCode.FAILURE;
            }

            if (HasView && viewFieldList != null &&
                    (EncodeViewRequest(encodeIter) < CodecReturnCode.SUCCESS))
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
            if (PrivateStream)
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

            foreach (string itemName in itemNames)
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

        private CodecReturnCode EncodeViewRequest(EncodeIterator encodeIter)
        {
            elementEntry.Clear();
            elementEntry.Name = ElementNames.VIEW_TYPE;
            elementEntry.DataType = DataTypes.UINT;
            tempUInt.Value(VIEW_TYPE);
            CodecReturnCode ret = elementEntry.Encode(encodeIter, tempUInt);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            elementEntry.Clear();
            elementEntry.Name = ElementNames.VIEW_DATA;
            elementEntry.DataType = DataTypes.ARRAY;
            if ((ret = elementEntry.EncodeInit(encodeIter, 0)) < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            viewArray.PrimitiveType = DataTypes.INT;
            viewArray.ItemLength = 2;

            if ((ret = viewArray.EncodeInit(encodeIter)) < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            foreach (string viewField in viewFieldList)
            {
                arrayEntry.Clear();
                tempInt.Value(int.Parse(viewField));
                ret = arrayEntry.Encode(encodeIter, tempInt);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            ret = viewArray.EncodeComplete(encodeIter, true);

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
            stringBuf.Insert(0, "MarketRequest: \n");
            stringBuf.Append(tab);
            stringBuf.Append("streaming: ");
            stringBuf.Append(Streaming);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("domain: ");
            stringBuf.Append(DomainTypes.ToString(DomainType));
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("isPrivateStream: ");
            stringBuf.Append(PrivateStream);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("hasView: ");
            stringBuf.Append(HasView);
            stringBuf.Append(eol);

            if (!HasIdentifier())
            {
                stringBuf.Append(tab);
                stringBuf.Append("itemNames: ");
                foreach(var name in ItemNames)
                {
                    stringBuf.Append(name);
                    stringBuf.Append(" ");
                }         
                stringBuf.Append(eol);
            }
            else
            {
                stringBuf.Append(tab);
                stringBuf.Append("Identifier: ");
                stringBuf.Append(Identifier);
                stringBuf.Append(eol);
            }

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
                stringBuf.Append(Qos.ToString());
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }

        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            return 0;
        }
        public override void Clear()
        {
            Flags = 0;
            qos.Clear();
            itemNames.Clear();
            PriorityClass = 1;
            PriorityCount = 1;
            Identifier = -1;
            viewFieldList.Clear();
            m_RequestMsg.Clear();
            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            m_RequestMsg.DomainType = (int)Rdm.DomainType.MARKET_PRICE;
            m_RequestMsg.ContainerType = DataTypes.NO_DATA;
        }
    }
}
