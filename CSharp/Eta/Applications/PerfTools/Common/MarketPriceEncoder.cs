/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Provides encoding of a MarketPrice domain data payload.
    /// </summary>
    public class MarketPriceEncoder
    {
        private const int TIM_TRK_1_FID = 3902; // Field TIM_TRK_1 is used to send update latency.
        private const int TIM_TRK_2_FID = 3903; // Field TIM_TRK_2 is used to send post latency.
        private const int TIM_TRK_3_FID = 3904; // Field TIM_TRK_3 is used to send generic msg latency.

        private XmlMsgData m_XmlMsgData;
        private FieldList m_FieldList = new FieldList();
        private FieldEntry m_FieldEntry = new FieldEntry();
        private UInt m_TempUInt = new UInt();

        public MarketPriceEncoder(XmlMsgData msgData) => m_XmlMsgData = msgData;

        /// <summary>
        /// Get the next MarketPrice generic message (moves over the list).
        /// </summary>
        /// <param name="mpItem">the market price item</param>
        /// <returns>the market price generic msg</returns>
        public MarketPriceMsg NextGenMsg(MarketPriceItem mpItem)
        {
            MarketPriceMsg mpGenMsg = m_XmlMsgData.MpGenMsgs![mpItem.IMsg++];
            if (mpItem.IMsg == m_XmlMsgData.GenMsgCount)
            {
                mpItem.IMsg = 0;
            }
            return mpGenMsg;
        }

        /// <summary>
        /// Get the next MarketPrice post (moves over the list).
        /// </summary>
        /// <param name="mpItem">the market price item</param>
        /// <returns>the market price post msg</returns>
        public MarketPriceMsg NextPostMsg(MarketPriceItem mpItem)
        {
            MarketPriceMsg mpPostMsg = m_XmlMsgData.MpPostMsgs![mpItem.IMsg++];
            if (mpItem.IMsg == m_XmlMsgData.PostCount)
            {
                mpItem.IMsg = 0;
            }              
            return mpPostMsg;
        }

        /// <summary>
        /// Encodes a MarketPrice data body for a message.
        /// </summary>
        /// <param name="_eIter">the EncodeIterator used for encoding</param>
        /// <param name="mpMsg">the market price message</param>
        /// <param name="msgClass">the class of the message</param>
        /// <param name="encodeStartTime">the encode start time</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation</returns>
        public CodecReturnCode EncodeDataBody(EncodeIterator _eIter, MarketPriceMsg mpMsg, int msgClass, long encodeStartTime)
        {
            m_FieldList.Clear();
            m_FieldEntry.Clear();
            m_FieldList.ApplyHasStandardData();
            CodecReturnCode codecReturnCode = m_FieldList.EncodeInit(_eIter, null, 0);
            if (codecReturnCode < CodecReturnCode.SUCCESS)
            {
                return codecReturnCode;
            }              
            for (int i = 0; i < mpMsg.FieldEntries.Length; ++i)
            {
                switch (mpMsg.FieldEntries[i].FieldEntry.DataType)
                {
                    case DataTypes.INT:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Int)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.UINT:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (UInt)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.FLOAT:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Float)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.DOUBLE:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Double)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.REAL:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Real)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.DATE:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Date)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.TIME:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Time)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.DATETIME:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (DateTime)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.QOS:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Qos)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.STATE:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (State)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.ENUM:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Enum)mpMsg.FieldEntries[i].Value!);
                        break;
                    case DataTypes.BUFFER:
                    case DataTypes.RMTES_STRING:
                        codecReturnCode = mpMsg.FieldEntries[i].FieldEntry.Encode(_eIter, (Buffer)mpMsg.FieldEntries[i].Value!);
                        break;
                    default:
                        codecReturnCode = CodecReturnCode.FAILURE;
                        break;
                }
                if (codecReturnCode < CodecReturnCode.SUCCESS)
                {
                    return codecReturnCode;
                }
                    
            }
            if (encodeStartTime > 0L)
            {
                FieldEntry fEntry = m_FieldEntry;
                int fid;
                switch (msgClass)
                {
                    case MsgClasses.UPDATE:
                        fid = TIM_TRK_1_FID;
                        break;
                    case MsgClasses.GENERIC:
                        fid = TIM_TRK_3_FID;
                        break;
                    default:
                        fid = TIM_TRK_2_FID;
                        break;
                }
                fEntry.FieldId = fid;

                m_FieldEntry.DataType = DataTypes.UINT;
                m_TempUInt.Value(encodeStartTime);
                codecReturnCode = m_FieldEntry.Encode(_eIter, m_TempUInt);
                if (codecReturnCode < CodecReturnCode.SUCCESS)
                {
                    return codecReturnCode;
                }
            }
            return m_FieldList.EncodeComplete(_eIter, true);
        }
    }
}
