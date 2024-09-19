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

namespace LSEG.Eta.Example.Common
{
    public class MarketByOrderRefresh : MarketByOrderBase
    {
        private Qos qos;
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

        private IRefreshMsg m_RefreshMsg = new Msg();
        
        public MarketByOrderRefresh()
        {
            qos = new Qos();
            Clear();
        }

        protected override bool IsRefreshType()
        {
            return true;
        }

        public override void Clear()
        {
            base.Clear();
            qos.Clear();
            Flags = 0;
            m_RefreshMsg.Clear();
            m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
            m_RefreshMsg.DomainType = (int)Rdm.DomainType.MARKET_BY_ORDER;
            m_RefreshMsg.ContainerType = DataTypes.MAP;
            ServiceId = 0;
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

            if (ClearCache)
            {
                m_RefreshMsg.ApplyClearCache();
            }

            m_RefreshMsg.ApplyHasMsgKey();

            if (HasServiceId)
            {
                m_RefreshMsg.MsgKey.ApplyHasServiceId();
                m_RefreshMsg.MsgKey.ServiceId = ServiceId;
            }

            if (PrivateStream)
            {
                m_RefreshMsg.ApplyPrivateStream();
            }

            m_RefreshMsg.MsgKey.ApplyHasName();
            m_RefreshMsg.MsgKey.ApplyHasNameType();

            m_RefreshMsg.MsgKey.Name = ItemName;
            m_RefreshMsg.MsgKey.NameType = InstrumentNameTypes.RIC;

            if (HasQos)
            {
                m_RefreshMsg.ApplyHasQos();
                qos.Copy(m_RefreshMsg.Qos);
            }

            return (Msg)m_RefreshMsg;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuffer();
            stringBuf.Insert(0, "MarketByOrderRefresh: \n");

            stringBuf.Append(tab);
            stringBuf.Append("state: ");
            stringBuf.Append(State);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("solicited: ");
            stringBuf.Append(Solicited);
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append("refreshComplete: ");
            stringBuf.Append(RefreshComplete);
            stringBuf.Append(eol);

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.Append(eol);
            }
            if (HasQos)
            {
                stringBuf.Append(tab);
                stringBuf.Append("qos: ");
                stringBuf.Append(m_RefreshMsg.Qos.ToString());
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }

        protected override CodecReturnCode EncodeSummaryData(EncodeIterator EncodeIter, DataDictionary dictionary, Map map)
        {
            // Encode fields for summary data
            fieldList.Clear();
            fieldList.ApplyHasStandardData();
            CodecReturnCode ret = fieldList.EncodeInit(EncodeIter, null, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            //CURRENCY 
            fieldEntry.Clear();
            IDictionaryEntry dictionaryEntry = dictionary.Entry(MarketByOrderItem.CURRENCY_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByOrderItem.CURRENCY_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                ret = fieldEntry.Encode(EncodeIter, MarketByOrderItem.USD_ENUM);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            //MARKET STATUS INDICATOR
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByOrderItem.MKT_STATUS_IND_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByOrderItem.MKT_STATUS_IND_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                ret = fieldEntry.Encode(EncodeIter, MarketByOrderItem.BBO_ENUM);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            //ACTIVE DATE
            fieldEntry.Clear();
            dictionaryEntry = dictionary.Entry(MarketByOrderItem.ACTIVE_DATE_FID);
            if (dictionaryEntry != null)
            {
                fieldEntry.FieldId = MarketByOrderItem.ACTIVE_DATE_FID;
                fieldEntry.DataType = dictionaryEntry.GetRwfType();
                tempDateTime.LocalTime();
                if ((ret =
                        fieldEntry.Encode(EncodeIter, tempDateTime.Date())) < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            return fieldList.EncodeComplete(EncodeIter, true);
        }

        protected override CodecReturnCode EncodeMapEntries(EncodeIterator EncodeIter, DataDictionary dictionary)
        {
            IDictionaryEntry dictionaryEntry;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            // Encode the order book
            foreach (OrderInfo orderInfo in MboInfo.OrderInfoList)
            {
                mapEntry.Clear();
                mapEntry.Action = MapEntryActions.ADD;
                tmpBuffer.Clear();
                tmpBuffer.Data(orderInfo.ORDER_ID);
                if ((ret = mapEntry.EncodeInit(EncodeIter, tmpBuffer, 0)) != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                fieldList.Clear();
                fieldList.ApplyHasStandardData();
                fieldList.ApplyHasSetData();
                fieldList.ApplyHasSetId();
                fieldList.SetId = 0;
                ret = fieldList.EncodeInit(EncodeIter, marketByOrderSetDefDb, 0);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                //Encode fields

                //ORDER_PRC
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketByOrderItem.ORDER_PRC_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketByOrderItem.ORDER_PRC_FID;
                    fieldEntry.DataType = dictionaryEntry.GetRwfType();
                    ret = fieldEntry.Encode(EncodeIter, orderInfo.ORDER_PRC);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                //ORDER_SIZE
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketByOrderItem.ORDER_SIZE_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketByOrderItem.ORDER_SIZE_FID;
                    fieldEntry.DataType = dictionaryEntry.GetRwfType();
                    ret = fieldEntry.Encode(EncodeIter, orderInfo.ORDER_SIZE);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                //QUOTIM_MS
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketByOrderItem.QUOTIM_MS_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketByOrderItem.QUOTIM_MS_FID;
                    fieldEntry.DataType = dictionaryEntry.GetRwfType();
                    tempUInt.Value(orderInfo.QUOTIM_MS);
                     // This encoding completes the encoding of the ORDER_PRC,
                     // ORDER_SIZE, QUOTIM_MS set.
                    ret = fieldEntry.Encode(EncodeIter, tempUInt);
                    if (ret != CodecReturnCode.SET_COMPLETE)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                }

                if (PrivateStream)
                {
                    //MKOASK_VOL
                    fieldEntry.Clear();
                    dictionaryEntry = dictionary.Entry(MarketByOrderItem.MKOASK_VOL_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.FieldId = MarketByOrderItem.MKOASK_VOL_FID;
                        fieldEntry.DataType = dictionaryEntry.GetRwfType();
                        ret = fieldEntry.Encode(EncodeIter, orderInfo.MKOASK_VOL);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }

                    //MKOBID_VOL
                    fieldEntry.Clear();
                    dictionaryEntry = dictionary.Entry(MarketByOrderItem.MKOBID_VOL_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.FieldId = MarketByOrderItem.MKOBID_VOL_FID;
                        fieldEntry.DataType = dictionaryEntry.GetRwfType();
                        ret = fieldEntry.Encode(EncodeIter, orderInfo.MKOBID_VOL);
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }
                }

                // Since the field list specified FieldListFlags.HAS_STANDARD_DATA
                // as well, standard entries can be Encoded after the set is
                // finished. The Add actions will additionally include the
                // ORDER_SIDE and MKT_MKR_ID fields.

                //ORDER_SIDE
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketByOrderItem.ORDER_SIDE_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketByOrderItem.ORDER_SIDE_FID;
                    fieldEntry.DataType = dictionaryEntry.GetRwfType();
                    ret = fieldEntry.Encode(EncodeIter, orderInfo.ORDER_SIDE);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                //MKT_MKR_ID 
                fieldEntry.Clear();
                dictionaryEntry = dictionary.Entry(MarketByOrderItem.MKT_MKR_ID_FID);
                if (dictionaryEntry != null)
                {
                    fieldEntry.FieldId = MarketByOrderItem.MKT_MKR_ID_FID;
                    fieldEntry.DataType = dictionaryEntry.GetRwfType();
                    tmpBuffer.Data(orderInfo.MKT_MKR_ID);
                    ret = fieldEntry.Encode(EncodeIter, tmpBuffer);
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

                ret = mapEntry.EncodeComplete(EncodeIter, true);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            return ret;
        }
    }
}
