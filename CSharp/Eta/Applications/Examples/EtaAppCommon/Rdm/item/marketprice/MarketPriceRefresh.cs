/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.Example.Common
{
    public class MarketPriceRefresh : MarketPriceBase
    {
        private Msg m_RefreshMsg = new Msg();
        private Qos m_Qos = new Qos();

        private UInt tmpUInt = new UInt();

        public override int StreamId { get => m_RefreshMsg.StreamId; set { m_RefreshMsg.StreamId = value; } }
        public override int MsgClass { get => m_RefreshMsg.MsgClass; }
        public override int DomainType { get => m_RefreshMsg.DomainType; }
        public bool Solicited
        {
            get => (Flags & ItemDomainCommonFlags.SOLICITED) != 0;
            set
            {
                if (value)
                {
                    Flags |= ItemDomainCommonFlags.SOLICITED;
                }
                else
                    Flags &= ~ItemDomainCommonFlags.SOLICITED;
            }
        }
        public bool RefreshComplete
        {
            get => (Flags & ItemDomainCommonFlags.REFRESH_COMPLETE) != 0;
            set
            {
                if (value)
                {
                    Flags |= ItemDomainCommonFlags.REFRESH_COMPLETE;
                }
                else
                    Flags &= ~ItemDomainCommonFlags.REFRESH_COMPLETE;
            }
        }
        public bool ClearCache 
        { 
            get => (Flags & ItemDomainCommonFlags.CLEAR_CACHE) != 0; 
            set 
            { 
                if (value) { Flags |= ItemDomainCommonFlags.CLEAR_CACHE; } 
                else Flags &= ~ItemDomainCommonFlags.CLEAR_CACHE; 
            } 
        }
        public bool HasQos
        {
            get => (Flags & ItemDomainCommonFlags.HAS_QOS) != 0;
            set
            {
                if (value)
                {
                    Flags |= ItemDomainCommonFlags.HAS_QOS;
                }
                else
                    Flags &= ~ItemDomainCommonFlags.HAS_QOS;
            }
        }
        public State State
        {
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
        public Qos Qos
        {
            get => m_Qos; 
            set 
            {
                Debug.Assert(value != null);
                value.Copy(m_Qos);
            }
        }
        public long Filter { get => m_RefreshMsg.MsgKey.Filter; set { m_RefreshMsg.MsgKey.Filter = value; } } 
        
        public MarketPriceRefresh()
        {
            Clear();
        }
  
        public override void Clear()
        {
            m_Qos.Clear();
            Flags = 0;
            m_RefreshMsg.Clear();
            m_RefreshMsg.DomainType = (int)Rdm.DomainType.MARKET_PRICE;
            m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
        }

        public override Msg EncodeMsg()
        {
            if (PrivateStream)
            {
                m_RefreshMsg.ApplyPrivateStream();
            }

            if (RefreshComplete)
            {
                m_RefreshMsg.ApplyRefreshComplete();
            }

            if (ClearCache)
            {
                m_RefreshMsg.ApplyClearCache();
            }

            if (Solicited)
            {
                m_RefreshMsg.ApplySolicited();
            }

            m_RefreshMsg.ApplyHasMsgKey();

            if (HasServiceId)
            {
                m_RefreshMsg.MsgKey.ApplyHasServiceId();
                m_RefreshMsg.MsgKey.ServiceId = ServiceId;
            }

            // Itemname
            m_RefreshMsg.MsgKey.Name = ItemName;
            m_RefreshMsg.MsgKey.ApplyHasName();
            m_RefreshMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            m_RefreshMsg.MsgKey.ApplyHasNameType();

            // State
            State.Copy(m_RefreshMsg.State);

            if (HasQos)
            {
                m_RefreshMsg.ApplyHasQos();
                Qos.Copy(m_RefreshMsg.Qos);
            }
            m_RefreshMsg.ContainerType = DataTypes.FIELD_LIST;

            return m_RefreshMsg;
        }

        public override string ToString()
        {
            StringBuilder toStringBuilder = PrepareStringBuilder();
            toStringBuilder.Insert(0, "MarketPriceRefresh: ");

            toStringBuilder.Append(tab);
            toStringBuilder.Append("state: ");
            toStringBuilder.Append(State);
            toStringBuilder.AppendLine();

            toStringBuilder.Append(tab);
            toStringBuilder.Append("solicited: ");
            toStringBuilder.Append(Solicited);
            toStringBuilder.AppendLine();

            toStringBuilder.Append(tab);
            toStringBuilder.Append("refreshComplete: ");
            toStringBuilder.Append(RefreshComplete);
            toStringBuilder.AppendLine();

            if (HasServiceId)
            {
                toStringBuilder.Append(tab);
                toStringBuilder.Append("serviceId: ");
                toStringBuilder.Append(ServiceId);
                toStringBuilder.AppendLine();
            }
            if (HasQos)
            {
                toStringBuilder.Append(tab);
                toStringBuilder.Append("qos: ");
                toStringBuilder.Append(m_RefreshMsg.Qos.ToString());
                toStringBuilder.AppendLine();
            }

            return toStringBuilder.ToString();
        }

        protected override CodecReturnCode EncodeRefreshFields(EncodeIterator encodeIter, DataDictionary dictionary)
        {
            MarketPriceItem mpItem = MarketPriceItem!;
            IDictionaryEntry dictionaryEntry;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            // RDNDISPLAY
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.RDNDISPLAY_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.RDNDISPLAY_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                tempUInt.Value(mpItem.RDNDISPLAY);
                if ((ret = fieldEntry.Encode(encodeIter, tempUInt)) < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            // RDN_EXCHID
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.RDN_EXCHID_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.RDN_EXCHID_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                Enum enumValue = new Enum();
                enumValue.Value(mpItem.RDN_EXCHID);
                if ((ret = fieldEntry.Encode(encodeIter, enumValue)) < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // DIVPAYDATE
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketPriceItem.DIVPAYDATE_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketPriceItem.DIVPAYDATE_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                return fieldEntry.Encode(encodeIter, mpItem.DIVPAYDATE);
            }

            return ret;
        }
    }

    
}
