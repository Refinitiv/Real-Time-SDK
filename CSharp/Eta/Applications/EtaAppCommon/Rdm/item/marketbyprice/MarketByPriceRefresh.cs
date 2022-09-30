/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace Refinitiv.Eta.Example.Common
{
    public class MarketByPriceRefresh : MarketByPriceBase
    {
        private Qos qos = new Qos();
        public int ServiceId { get; set; }

        public override int StreamId { get => m_RefreshMsg.StreamId; set { m_RefreshMsg.StreamId = value; } }
        public override int MsgClass { get => m_RefreshMsg.MsgClass; }
        public override int DomainType { get => m_RefreshMsg.DomainType; }

        public ItemDomainCommonFlags Flags { get; set; }

        public bool Solicited
        {
            get => (Flags & ItemDomainCommonFlags.SOLICITED) != 0;
            set
            {
                if (value)
                {
                    Flags |= ItemDomainCommonFlags.SOLICITED;
                    m_RefreshMsg.ApplySolicited();
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
        public bool HasServiceId
        {
            get => (Flags & ItemDomainCommonFlags.HAS_SERVICE_ID) != 0;
            set
            {
                if (value)
                {
                    Flags |= ItemDomainCommonFlags.HAS_SERVICE_ID;
                }
                else
                    Flags &= ~ItemDomainCommonFlags.HAS_SERVICE_ID;
            }
        }
        public bool PrivateStream
        {
            get => (Flags & ItemDomainCommonFlags.PRIVATE_STREAM) != 0;
            set
            {
                if (value)
                {
                    Flags |= ItemDomainCommonFlags.PRIVATE_STREAM;
                }
                else
                    Flags &= ~ItemDomainCommonFlags.SOLICITED;
            }
        }
        public State State
        {
            get => m_RefreshMsg.State;
            set
            {
                Debug.Assert(value != null);
                value.Copy(m_RefreshMsg.State);
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
        public long Filter { get => m_RefreshMsg.MsgKey.Filter; set { m_RefreshMsg.MsgKey.Filter = value; } }

        private IRefreshMsg m_RefreshMsg = new Msg();

        public MarketByPriceRefresh()
        {
            Clear();
        }

        public override Msg EncodeMsg()
        {
            if (RefreshComplete)
            {
                m_RefreshMsg.ApplyRefreshComplete();
            }

            if (Solicited)
            {
                m_RefreshMsg.ApplySolicited();
            }

            if (PrivateStream)
            {
                m_RefreshMsg.ApplyPrivateStream();
            }

            if (ClearCache)
            {
                m_RefreshMsg.ApplyClearCache();
            }

            if (HasServiceId)
            {
                m_RefreshMsg.MsgKey.ApplyHasServiceId();
                m_RefreshMsg.MsgKey.ServiceId = ServiceId;
            }
            m_RefreshMsg.MsgKey.ApplyHasName();
            m_RefreshMsg.MsgKey.ApplyHasNameType();

            /* Itemname */
            m_RefreshMsg.MsgKey.Name = ItemName;
            m_RefreshMsg.MsgKey.NameType = InstrumentNameTypes.RIC;

            if (HasQos)
            {
                m_RefreshMsg.ApplyHasQos();
                qos.Copy(m_RefreshMsg.Qos);
            }

            return (Msg)m_RefreshMsg;
        }

        protected override CodecReturnCode EncodeMapEntries(EncodeIterator EncodeIter, DataDictionary dictionary)
        {
            // Encode the order book
            PriceInfo priceInfo = MbpInfo.PriceInfoList[PartNo];
            mapEntry.Clear();
            mapEntry.Action = MapEntryActions.ADD;
            tmpBuffer.Data(priceInfo.PRICE_POINT);
            CodecReturnCode ret;
            if ((ret = mapEntry.EncodeInit(EncodeIter, tmpBuffer, 0)) != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            fieldList.Clear();
            fieldList.ApplyHasStandardData();
            fieldList.ApplyHasSetData();
            fieldList.ApplyHasSetId();
            fieldList.SetId = 0;

            ret = fieldList.EncodeInit(EncodeIter, marketByPriceSetDefDb, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            // Encode fields

            // ORDER_PRC
            fieldEntry.Clear();
            IDictionaryEntry dictionaryEntry = dictionary.Entry(MarketByPriceItem.ORDER_PRC_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByPriceItem.ORDER_PRC_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                ret = fieldEntry.Encode(EncodeIter, priceInfo.ORDER_PRC);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // ORDER_SIZE
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByPriceItem.ORDER_SIZE_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByPriceItem.ORDER_SIZE_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                ret = fieldEntry.Encode(EncodeIter, priceInfo.ORDER_SIZE);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // QUOTIM_MS
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByPriceItem.QUOTIM_MS_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByPriceItem.QUOTIM_MS_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                tempUInt.Value(priceInfo.QUOTIM_MS);
                ret = fieldEntry.Encode(EncodeIter, tempUInt);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // NO_ORD
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByPriceItem.NO_ORD_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByPriceItem.NO_ORD_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                tempUInt.Value(priceInfo.NO_ORD);

                // This encoding completes the encoding of the ORDER_PRC,
                // ORDER_SIZE, QUOTIM_MS, NO_ORD set.
                ret = fieldEntry.Encode(EncodeIter, tempUInt);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return CodecReturnCode.FAILURE;
                }
            }

            // Encode items for private stream
            if (PrivateStream)
            {
                // MKOASK_VOL
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketByPriceItem.MKOASK_VOL_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketByPriceItem.MKOASK_VOL_FID;
                    fieldEntry.DataType = dictionaryEntry.RwfType;
                    ret = fieldEntry.Encode(EncodeIter, priceInfo.MKOASK_VOL);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                // MKOBID_VOL
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketByPriceItem.MKOBID_VOL_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketByPriceItem.MKOBID_VOL_FID;
                    fieldEntry.DataType = dictionaryEntry.RwfType;
                    ret = fieldEntry.Encode(EncodeIter, priceInfo.MKOBID_VOL);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }
            /*
             * Since the field list specified FieldListFlags.HAS_STANDARD_DATA as
             * well, standard entries can be Encoded after the set is finished. The
             * Add actions will additionally include the ORDER_SIDE field.
             */
            // ORDER_SIDE
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByPriceItem.ORDER_SIDE_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByPriceItem.ORDER_SIDE_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                ret = fieldEntry.Encode(EncodeIter, priceInfo.ORDER_SIDE);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            ret = fieldList.EncodeComplete(EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return mapEntry.EncodeComplete(EncodeIter, true);
        }
        protected override CodecReturnCode EncodeSummaryData(EncodeIterator EncodeIter, DataDictionary dictionary, Map map)
        {
            IDictionaryEntry dictionaryEntry;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            /* Encode fields for summary data */
            fieldList.Clear();
            fieldList.ApplyHasStandardData();
            if ((ret = fieldList.EncodeInit(EncodeIter, null, 0)) < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            /* CURRENCY */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByPriceItem.CURRENCY_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByOrderItem.CURRENCY_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                if ((ret = fieldEntry.Encode(EncodeIter, MarketByOrderItem.USD_ENUM)) < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            /* MARKET STATUS INDICATOR */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByOrderItem.MKT_STATUS_IND_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByOrderItem.MKT_STATUS_IND_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                if ((ret = fieldEntry.Encode(EncodeIter, MarketByOrderItem.BBO_ENUM)) < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            /* ACTIVE DATE */
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByOrderItem.ACTIVE_DATE_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByOrderItem.ACTIVE_DATE_FID;
                fieldEntry.DataType = dictionaryEntry.RwfType;
                tempDateTime.LocalTime();
                if ((ret = fieldEntry.Encode(EncodeIter, tempDateTime.Date())) < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            ret = fieldList.EncodeComplete(EncodeIter, true);
            return ret;
        }

        public override void Clear()
        {
            base.Clear();
            m_RefreshMsg.Clear();
            m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
            m_RefreshMsg.DomainType = (int)Rdm.DomainType.MARKET_BY_PRICE;
            m_RefreshMsg.ContainerType = DataTypes.MAP;
            m_RefreshMsg.ApplyHasMsgKey();
            qos.Clear();
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "MarketByPriceRefresh: \n");

            stringBuf.Append(tabChar);
            stringBuf.Append("state: ");
            stringBuf.Append(State);
            stringBuf.Append(eolChar);

            stringBuf.Append(tabChar);
            stringBuf.Append("solicited: ");
            stringBuf.Append(Solicited);
            stringBuf.Append(eolChar);

            stringBuf.Append(tabChar);
            stringBuf.Append("refreshComplete: ");
            stringBuf.Append(RefreshComplete);
            stringBuf.Append(eolChar);

            if (HasServiceId)
            {
                stringBuf.Append(tabChar);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.Append(eolChar);
            }
            if (HasQos)
            {
                stringBuf.Append(tabChar);
                stringBuf.Append("qos: ");
                stringBuf.Append(qos.ToString());
                stringBuf.Append(eolChar);
            }

            return stringBuf.ToString();
        }
    }
}
