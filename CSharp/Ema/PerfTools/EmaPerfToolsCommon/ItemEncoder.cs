/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Eta.Codec;
using System.Text;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.PerfTools.Common
{
    public class ItemEncoder
    {
        /// <summary>
        /// Field ID used to send update latency.
        /// </summary>
        public const int TIM_TRK_1_FID = 3902;

        /// <summary>
        /// Field ID used to send post latency.
        /// </summary>
        public const int TIM_TRK_2_FID = 3903;

        /// <summary>
        /// Field ID used to send generic msg latency.
        /// </summary>
        public const int TIM_TRK_3_FID = 3904;

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
            switch (field.LoadType)
            {
                case DataType.DataTypes.INT:
                    if (!field.Blank)
                    {
                        Int intValue = (Int)field.FieldEntry!;
                        fieldList.AddInt(fieldId, intValue.ToLong());
                    }
                    else
                    {
                        fieldList.AddCodeInt(fieldId);
                    }
                    break;
                case DataType.DataTypes.UINT:
                    if (!field.Blank)
                    {
                        UInt uintValue = (UInt)field.FieldEntry!;
                        fieldList.AddUInt(fieldId, uintValue.ToULong());
                    }
                    else
                    {
                        fieldList.AddCodeUInt(fieldId);
                    }
                    break;
                case DataType.DataTypes.FLOAT:
                    if (!field.Blank)
                    {
                        Float floatValue = (Float)field.FieldEntry!;
                        fieldList.AddFloatValue(fieldId, floatValue.ToFloat());
                    }
                    else
                    {
                        fieldList.AddCodeFloat(fieldId);
                    }
                    break;
                case DataType.DataTypes.DOUBLE:
                    if (!field.Blank)
                    {
                        Double doubleValue = (Double)field.FieldEntry!;
                        fieldList.AddDoubleValue(fieldId, doubleValue.ToDouble());
                    }
                    else
                    {
                        fieldList.AddCodeDouble(fieldId);
                    }
                    break;
                case DataType.DataTypes.REAL:
                    if (!field.Blank)
                    {
                        Real realValue = (Real)field.FieldEntry!;
                        fieldList.AddReal(fieldId, realValue.ToLong(), realValue.Hint);
                    }
                    else
                    {
                        fieldList.AddCodeReal(fieldId);
                    }
                    break;
                case DataType.DataTypes.DATE:
                    if (!field.Blank)
                    {
                        Date dateTmp = (Date)field.FieldEntry!;
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
                        Time timeTmp = (Time)field.FieldEntry!;
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
                        DateTime dateTimeTmp = (DateTime)field.FieldEntry!;
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
                        Enum enumValue = (Enum)field.FieldEntry!;
                        fieldList.AddEnumValue(fieldId, enumValue.ToInt());
                    }
                    else
                    {
                        fieldList.AddCodeEnum(fieldId);
                    }
                    break;
                case DataType.DataTypes.BUFFER:
                    if (!field.Blank)
                    {
                        EmaBuffer bufferTmp = (EmaBuffer)field.FieldEntry!;
                        fieldList.AddBuffer(fieldId, bufferTmp);
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
                        EmaBuffer bufferTmp = (EmaBuffer)field.FieldEntry!;
                        fieldList.AddRmtes(fieldId, bufferTmp);
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

        public void CreatePayload(MarketPriceMsg mpMsg, long encodeStartTimeMicroseconds, int timeFieldId, Ema.Access.FieldList fieldList, bool setLatency = true)
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
            else if (encodeStartTimeMicroseconds > 0 && setLatency)
            {
                fieldList.AddUInt(timeFieldId, (ulong)encodeStartTimeMicroseconds);
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
