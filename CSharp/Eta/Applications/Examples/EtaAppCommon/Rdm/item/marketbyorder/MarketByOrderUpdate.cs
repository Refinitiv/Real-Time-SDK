/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Text;

namespace LSEG.Eta.Example.Common
{
    public class MarketByOrderUpdate : MarketByOrderBase
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

        public MarketByOrderUpdate()
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
            m_UpdateMsg.DomainType = (int)Rdm.DomainType.MARKET_BY_ORDER;
            m_UpdateMsg.ContainerType = DataTypes.MAP;
        }

        public override Msg EncodeMsg()
        {
            if (HasServiceId)
            {
                m_UpdateMsg.ApplyHasMsgKey();
                m_UpdateMsg.MsgKey.ApplyHasServiceId();
                m_UpdateMsg.MsgKey.ServiceId = ServiceId;

                m_UpdateMsg.MsgKey.ApplyHasName();
                m_UpdateMsg.MsgKey.Name = ItemName;

                m_UpdateMsg.MsgKey.ApplyHasNameType();
                m_UpdateMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            }

            return (Msg)m_UpdateMsg;
        }

        protected override CodecReturnCode EncodeMapEntries(EncodeIterator encIter, DataDictionary dictionary)
        {
            IDictionaryEntry dictionaryEntry;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            // encode the order book
            mapEntry.Clear();

            int idx = -1;
            foreach (OrderInfo orderInfo in MboInfo.OrderInfoList)
            {
                ++idx;

                if (orderInfo.Lifetime != 0)
                {
                    // determine if this is an ADD or UPDATE. This logic matches the
                    // logic used in MarketByOrderItem.
                    if (orderInfo.Lifetime == (5 + idx))
                        mapEntry.Action = MapEntryActions.ADD;
                    else
                        mapEntry.Action = MapEntryActions.UPDATE;

                    tmpBuffer.Data(orderInfo.ORDER_ID);
                    if ((ret = mapEntry.EncodeInit(encIter, tmpBuffer, 0)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    fieldList.Clear();
                    fieldList.ApplyHasStandardData();

                    fieldList.ApplyHasSetData();
                    fieldList.ApplyHasSetId();
                    fieldList.SetId = 0;

                    if ((ret = fieldList.EncodeInit(encIter, marketByOrderSetDefDb, 0)) < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    //encode fields

                    //ORDER_PRC
                    fieldEntry.Clear();
                    dictionaryEntry = dictionary.Entry(MarketByOrderItem.ORDER_PRC_FID);
                    if (dictionaryEntry != null)
                    {
                        fieldEntry.FieldId = MarketByOrderItem.ORDER_PRC_FID;
                        fieldEntry.DataType = dictionaryEntry.GetRwfType();
                        if ((ret = fieldEntry.Encode(encIter, orderInfo.ORDER_PRC)) < CodecReturnCode.SUCCESS)
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
                        if ((ret = fieldEntry.Encode(encIter, orderInfo.ORDER_SIZE)) < CodecReturnCode.SUCCESS)
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
                        /*
                         * This encoding completes the encoding of the ORDER_PRC,
                         * ORDER_SIZE, QUOTIM_MS set. When a set is completed, a success
                         * code, CodecReturnCode.SET_COMPLETE, is returned to indicate
                         * this. This may be used to ensure that the set is being used
                         * as intended.
                         */
                        ret = fieldEntry.Encode(encIter, tempUInt);
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
                            if ((ret = fieldEntry.Encode(encIter, orderInfo.MKOASK_VOL)) < CodecReturnCode.SUCCESS)
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
                            if ((ret = fieldEntry.Encode(encIter, orderInfo.MKOBID_VOL)) < CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }
                    }


                    /*
                     * Since the field list specified FieldListFlags.HAS_STANDARD_DATA as
                     * well, standard entries can be encoded after the set is finished.
                     * The Add actions will additionally include the ORDER_SIDE and
                     * MKT_MKR_ID fields.
                     */

                    if (orderInfo.Lifetime == (5 + idx))
                    {
                        //ORDER_SIDE
                        fieldEntry.Clear();
                        dictionaryEntry = dictionary.Entry(MarketByOrderItem.ORDER_SIDE_FID);
                        if (dictionaryEntry != null)
                        {
                            fieldEntry.FieldId = MarketByOrderItem.ORDER_SIDE_FID;
                            fieldEntry.DataType = dictionaryEntry.GetRwfType();
                            if ((ret = fieldEntry.Encode(encIter, orderInfo.ORDER_SIDE)) < CodecReturnCode.SUCCESS)
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
                            if ((ret = fieldEntry.Encode(encIter, tmpBuffer)) < CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }
                        }
                    }
                    if ((ret = fieldList.EncodeComplete(encIter, true)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    if ((ret = mapEntry.EncodeComplete(encIter, true)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                else
                {
                    //delete the order
                    mapEntry.Action = MapEntryActions.DELETE;
                    tmpBuffer.Data(orderInfo.ORDER_ID);
                    if ((ret = mapEntry.Encode(encIter, tmpBuffer)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            return ret;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuffer();
            stringBuf.Insert(0, "MarketByOrderUpdate: \n");

            if (HasServiceId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("serviceId: ");
                stringBuf.Append(ServiceId);
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }
    }
}
