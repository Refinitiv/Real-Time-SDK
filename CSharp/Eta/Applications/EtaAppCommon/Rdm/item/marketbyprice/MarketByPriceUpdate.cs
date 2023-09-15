/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Text;

namespace LSEG.Eta.Example.Common
{
    public class MarketByPriceUpdate : MarketByPriceBase
    {
        private IUpdateMsg m_UpdateMsg = new Msg();
        public int ServiceId { get; set; }
        public override int StreamId { get => m_UpdateMsg.StreamId; set { m_UpdateMsg.StreamId = value; } }
        public override int MsgClass { get => m_UpdateMsg.MsgClass; }
        public override int DomainType { get => m_UpdateMsg.DomainType; }

        public ItemDomainCommonFlags Flags { get; set; }

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

        public MarketByPriceUpdate()
        {
            Clear();
        }

        public override void Clear()
        {
            base.Clear();
            Flags = 0;
            ServiceId = 0;
            m_UpdateMsg.Clear();
            m_UpdateMsg.MsgClass = MsgClasses.UPDATE;
            m_UpdateMsg.DomainType = (int)Rdm.DomainType.MARKET_BY_PRICE;
            m_UpdateMsg.ContainerType = DataTypes.MAP;
        }

        public override Msg EncodeMsg()
        {
            if (HasServiceId)
            {
                m_UpdateMsg.ApplyHasMsgKey();
                m_UpdateMsg.MsgKey.ServiceId = ServiceId;
                m_UpdateMsg.MsgKey.ApplyHasServiceId();
                m_UpdateMsg.MsgKey.Name = ItemName;
                m_UpdateMsg.MsgKey.ApplyHasName();
                m_UpdateMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
                m_UpdateMsg.MsgKey.ApplyHasNameType();
            }

            return (Msg)m_UpdateMsg;
        }

        protected override CodecReturnCode EncodeMapEntries(EncodeIterator EncodeIter, DataDictionary dictionary)
        {
            IDictionaryEntry dictionaryEntry;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            //Encode the order book
            mapEntry.Clear();

            int idx = -1;
            foreach (PriceInfo priceInfo in MbpInfo.PriceInfoList)
            {
                ++idx;

                if (priceInfo.Lifetime != 0)
                {
                    // determine if this is an ADD or UPDATE. This logic matches the
                    // logic used in MarketByOrderItem.
                    if (priceInfo.Lifetime == (5 + idx) && RefreshComplete)
                        mapEntry.Action = MapEntryActions.ADD;
                    else
                        mapEntry.Action = MapEntryActions.UPDATE;

                    tmpBuffer.Data(priceInfo.PRICE_POINT);
                    if ((ret = mapEntry.EncodeInit(EncodeIter, tmpBuffer, 0)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    fieldList.Clear();
                    fieldList.ApplyHasStandardData();

                    fieldList.ApplyHasSetData();
                    fieldList.ApplyHasSetId();
                    fieldList.SetId = 0;

                    if ((ret = fieldList.EncodeInit(EncodeIter, marketByPriceSetDefDb, 0)) < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    //Encode fields

                    //ORDER_PRC
                    fieldEntry.Clear();
                    dictionaryEntry = dictionary.Entry(MarketByPriceItem.ORDER_PRC_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.FieldId = MarketByPriceItem.ORDER_PRC_FID;
                        fieldEntry.DataType = dictionaryEntry.GetRwfType();
                        if ((ret = fieldEntry.Encode(EncodeIter, priceInfo.ORDER_PRC)) < CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }

                    //ORDER_SIZE
                    fieldEntry.Clear();
                    dictionaryEntry = dictionary.Entry(MarketByPriceItem.ORDER_SIZE_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.FieldId = MarketByPriceItem.ORDER_SIZE_FID;
                        fieldEntry.DataType = dictionaryEntry.GetRwfType();
                        if ((ret = fieldEntry.Encode(EncodeIter, priceInfo.ORDER_SIZE)) < CodecReturnCode.SUCCESS)
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
                        fieldEntry.DataType = dictionaryEntry.GetRwfType();
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
                        fieldEntry.DataType = dictionaryEntry.GetRwfType();
                        tempUInt.Value(priceInfo.NO_ORD);
                        // This encoding completes the encoding of the ORDER_PRC,
                        // ORDER_SIZE, QUOTIM_MS, NO_ORD set. 
                        ret = fieldEntry.Encode(EncodeIter, tempUInt);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            return CodecReturnCode.FAILURE;
                        }
                    }


                    //Encode items for private stream
                    if (PrivateStream)
                    {
                        //MKOASK_VOL
                        fieldEntry.Clear();
                        dictionaryEntry = dictionary.Entry(MarketByPriceItem.MKOASK_VOL_FID);
                        if (dictionaryEntry != null)
                        {
                            fieldEntry.FieldId = MarketByPriceItem.MKOASK_VOL_FID;
                            fieldEntry.DataType = dictionaryEntry.GetRwfType();
                            if ((ret = fieldEntry.Encode(EncodeIter, priceInfo.MKOASK_VOL)) < CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }

                        //MKOBID_VOL
                        fieldEntry.Clear();
                        dictionaryEntry = dictionary.Entry(MarketByPriceItem.MKOBID_VOL_FID);
                        if (dictionaryEntry != null)
                        {
                            fieldEntry.FieldId = MarketByPriceItem.MKOBID_VOL_FID;
                            fieldEntry.DataType = dictionaryEntry.GetRwfType();
                            if ((ret = fieldEntry.Encode(EncodeIter, priceInfo.MKOBID_VOL)) < CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }
                    }

                    // Since the field list specified
                    // FieldListFlags.HAS_STANDARD_DATA as well, standard entries
                    // can be Encoded after the set is finished. The Add actions
                    // will additionally include the ORDER_SIDE and MKT_MKR_ID
                    // fields.
                    if (priceInfo.Lifetime == (5 + idx) && RefreshComplete)
                    {
                        //ORDER_SIDE
                        fieldEntry.Clear();
                        dictionaryEntry = dictionary.Entry(MarketByPriceItem.ORDER_SIDE_FID);
                        if (dictionaryEntry != null)
                        {
                            fieldEntry.FieldId = MarketByPriceItem.ORDER_SIDE_FID;
                            fieldEntry.DataType = dictionaryEntry.GetRwfType();
                            if ((ret = fieldEntry.Encode(EncodeIter, priceInfo.ORDER_SIDE)) < CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }
                    }
                    if ((ret = fieldList.EncodeComplete(EncodeIter, true)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    if ((ret = mapEntry.EncodeComplete(EncodeIter, true)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                else
                {
                    //delete the order
                    mapEntry.Action = MapEntryActions.DELETE;
                    tmpBuffer.Data(priceInfo.PRICE_POINT);
                    if ((ret = mapEntry.Encode(EncodeIter, tmpBuffer)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            return ret;
        }

        public override string ToString()
        {
            StringBuilder toStringBuilder = PrepareStringBuilder();
            toStringBuilder.Insert(0, "MarketByPriceUpdate: ");

            if (HasServiceId)
            {
                toStringBuilder.Append(tabChar);
                toStringBuilder.Append("serviceId: ");
                toStringBuilder.Append(ServiceId);
                toStringBuilder.Append(eolChar);
            }

            return toStringBuilder.ToString();

        }
    }
}
