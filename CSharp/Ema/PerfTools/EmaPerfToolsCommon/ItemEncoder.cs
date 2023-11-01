/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Eta.Codec;
using System.Text;
using DateTime = LSEG.Eta.Codec.DateTime;

namespace LSEG.Ema.PerfTools.Common
{
    public class ItemEncoder
    {
        private const int TIM_TRK_1_FID = 3902; // Field TIM_TRK_1 is used to send update latency.
        private const int TIM_TRK_2_FID = 3903; // Field TIM_TRK_2 is used to send post latency.
        private const int TIM_TRK_3_FID = 3904; // Field TIM_TRK_3 is used to send generic msg latency.

        private Date dateTmp = new Date();
        private Time timeTmp = new Time();
        private DateTime dateTimeTmp = new DateTime();

        private Ema.Access.FieldList m_fieldList = new Ema.Access.FieldList();
        private Ema.Access.UpdateMsg m_updateMsg = new Ema.Access.UpdateMsg();

        private XmlMsgData _xmlMsgData;

        public ItemEncoder(XmlMsgData xmlMsgData)
        {
            _xmlMsgData = xmlMsgData;
        }

        /// <summary>
        /// Creates field entry data from file data.
        /// </summary>
        /// <param name="fieldList">FieldList to add entry to</param>
        /// <param name="field">MarketField instance containing data</param>
        public void LoadPrimitive(Ema.Access.FieldList fieldList, MarketField field)
        {
            int fieldId = field.FieldId;
            string value = field.Value!;
            double doubleValue;
            float floatValue;
            long longValue;
            int intValue;
            switch (field.LoadType)
            {
                case DataType.DataTypes.INT:
                    if (!field.Blank)
                    {
                        long.TryParse(value, out longValue);
                        fieldList.AddInt(fieldId, longValue);
                    }
                    else
                    {
                        fieldList.AddCodeInt(fieldId);
                    }
                    break;
                case DataType.DataTypes.UINT:
                    if (!field.Blank)
                    {
                        long.TryParse(value, out longValue);
                        fieldList.AddUInt(fieldId, (ulong)longValue);
                    }
                    else
                    {
                        fieldList.AddCodeUInt(fieldId);
                    }
                    break;
                case DataType.DataTypes.FLOAT:
                    if (!field.Blank)
                    {
                        float.TryParse(value, out floatValue);
                        fieldList.AddFloatValue(fieldId, floatValue);
                    }
                    else
                    {
                        fieldList.AddCodeFloat(fieldId);
                    }
                    break;
                case DataType.DataTypes.DOUBLE:
                    if (!field.Blank)
                    {
                        double.TryParse(value, out doubleValue);
                        fieldList.AddDoubleValue(fieldId, doubleValue);
                    }
                    else
                    {
                        fieldList.AddCodeDouble(fieldId);
                    }
                    break;
                case DataType.DataTypes.REAL:
                    if (!field.Blank)
                    {
                        double.TryParse(value, out doubleValue);
                        fieldList.AddRealFromDouble(fieldId, doubleValue);
                    }
                    else
                    {
                        fieldList.AddCodeReal(fieldId);
                    }
                    break;
                case DataType.DataTypes.DATE:
                    if (!field.Blank)
                    {
                        dateTmp.Clear();
                        dateTmp.Value(value);
                        fieldList.AddDate(fieldId, dateTmp.Year(), dateTmp.Month(), dateTmp.Day());
                    }
                    else
                    {
                        fieldList.AddCodeDate(fieldId);
                    }
                    break;
                case DataType.DataTypes.TIME:
                    if (!field.Blank)
                    {
                        timeTmp.Clear();
                        timeTmp.Value(value);
                        fieldList.AddTime(fieldId, timeTmp.Hour(), 
                            timeTmp.Minute(), timeTmp.Second(), 
                            timeTmp.Millisecond(), timeTmp.Microsecond(), timeTmp.Nanosecond());
                    }
                    else
                    {
                        fieldList.AddCodeTime(fieldId);
                    }
                    break;
                case DataType.DataTypes.DATETIME:
                    if (!field.Blank)
                    {
                        dateTimeTmp.Clear();
                        dateTimeTmp.Value(value);
                        fieldList.AddDateTime(fieldId, dateTimeTmp.Year(), 
                            dateTimeTmp.Month(), dateTimeTmp.Day(), 
                            dateTimeTmp.Hour(), dateTimeTmp.Minute(), dateTimeTmp.Second(), 
                            dateTimeTmp.Millisecond(), dateTimeTmp.Microsecond(), dateTimeTmp.Nanosecond());
                    }
                    else
                    {
                        fieldList.AddCodeDateTime(fieldId);
                    }
                    break;
                case DataType.DataTypes.ENUM:
                    if (!field.Blank)
                    {
                        int.TryParse(value, out intValue);
                        fieldList.AddEnumValue(fieldId, intValue);
                    }
                    else
                    {
                        fieldList.AddCodeEnum(fieldId);
                    }
                    break;
                case DataType.DataTypes.BUFFER:
                    if (!field.Blank)
                    {
                        fieldList.AddBuffer(fieldId, new EmaBuffer(Encoding.ASCII.GetBytes(value!)));
                    }
                    else
                    {
                        fieldList.AddCodeBuffer(fieldId);
                    }
                    break;
                case DataType.DataTypes.ASCII:
                    if (!field.Blank)
                    {
                        fieldList.AddAscii(fieldId, value!);
                    }
                    else
                    {
                        fieldList.AddCodeAscii(fieldId);
                    }
                    break;
                case DataType.DataTypes.UTF8:
                    if (!field.Blank)
                    {
                        fieldList.AddUtf8(fieldId, value!);
                    }
                    else
                    {
                        fieldList.AddCodeUtf8(fieldId);
                    }
                    break;
                case DataType.DataTypes.RMTES:
                    if (!field.Blank)
                    {
                        fieldList.AddRmtes(fieldId, new EmaBuffer(Encoding.ASCII.GetBytes(value!)));
                    }
                    else
                    {
                        fieldList.AddCodeRmtes(fieldId);
                    }
                    break;
                case DataType.DataTypes.QOS:
                    var qos = field.Qos;
                    fieldList.AddQos(fieldId, (uint)qos!.Timeliness(), (uint)qos.Rate());
                    break;
                case DataType.DataTypes.STATE:
                    var state = field.State;
                    fieldList.AddState(fieldId, state!.StreamState(), state.DataState(), state.StatusCode(), state!.StatusText()!);
                    break;
                default:
                    break;
            }
        }

        private void CreatePayload(MarketPriceMsg mpMsg, long encodeStartTime, int timeFieldId, Ema.Access.FieldList fieldList)
        {
            for (int i = 0; i < mpMsg.FieldEntryCount; i++)
            {
                LoadPrimitive(fieldList, mpMsg.FieldEntries[i]);
            }

            if (timeFieldId == 0)
            {
                fieldList.AddUInt(TIM_TRK_1_FID, 0L);
                fieldList.AddUInt(TIM_TRK_2_FID, 0L);
                fieldList.AddUInt(TIM_TRK_3_FID, 0L);
            }
            else if (encodeStartTime > 0)
            {
                fieldList.AddUInt(timeFieldId, (ulong)encodeStartTime);
            }

            fieldList.Complete();
        }

        public MarketPriceMsg NextGenMsg(MarketPriceItem mpItem)
        {
            int mpItemIndex = mpItem.IMsg;
            MarketPriceMsg mpMsg = _xmlMsgData.MpGenMsgs![mpItemIndex++];

            if (mpItemIndex == _xmlMsgData.GenMsgCount)
                mpItemIndex = 0;

            mpItem.IMsg = mpItemIndex;

            return mpMsg;
        }

        public MarketPriceMsg NextPostMsg(MarketPriceItem mpItem)
        {
            int mpItemIndex = mpItem.IMsg;
            MarketPriceMsg mpMsg = _xmlMsgData.MpPostMsgs![mpItemIndex++];

            if (mpItemIndex == _xmlMsgData.PostCount)
                mpItemIndex = 0;

            mpItem.IMsg = mpItemIndex;

            return mpMsg;
        }

        public CodecReturnCode PopulateGenericMsg(GenericMsg genMsg, ItemInfo itemInfo, long encodeStartTime)
        {
            genMsg.StreamId(itemInfo.ItemId);
            genMsg.DomainType(itemInfo.Attributes.DomainType);
            switch (itemInfo.Attributes.DomainType)
            {
                case (int)Eta.Rdm.DomainType.MARKET_PRICE:
                    m_fieldList.Clear();
                    CreatePayload(NextGenMsg(itemInfo.MarketPriceItem!), encodeStartTime, TIM_TRK_3_FID, m_fieldList);
                    genMsg.Payload();
                    break;
                default:
                    return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.SUCCESS;
        }

        public CodecReturnCode PopulatePostMsg(PostMsg postMsg, ItemInfo itemInfo, PostUserInfo postUserInfo, long encodeStartTime)
        {
            postMsg.StreamId(itemInfo.ItemId);
            switch (itemInfo.Attributes.DomainType)
            {
                case (int)Eta.Rdm.DomainType.MARKET_PRICE:
                    m_updateMsg.Clear();
                    m_fieldList.Clear();
                    CreatePayload(NextPostMsg(itemInfo.MarketPriceItem!), encodeStartTime, TIM_TRK_2_FID, m_fieldList);
                    m_updateMsg.Payload(m_fieldList).PublisherId(postUserInfo.UserId, postUserInfo.UserAddr);
                    postMsg.PublisherId(postUserInfo.UserId, postUserInfo.UserAddr).Payload(m_updateMsg).Complete(true);
                    break;
                default:
                    return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.SUCCESS;
        }
    }
}
