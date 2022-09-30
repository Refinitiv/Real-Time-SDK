/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using System;
using Buffer = Refinitiv.Eta.Codec.Buffer;
using DateTime = Refinitiv.Eta.Codec.DateTime;
using Double = Refinitiv.Eta.Codec.Double;
using Enum = Refinitiv.Eta.Codec.Enum;

namespace Refinitiv.Eta.PerfTools.Common
{
    /// <summary>
    /// The market price decoder for the ConsPerf application.
    /// </summary>
    public class MarketPriceDecoder
    {
        private const int TIM_TRK_1_FID = 3902;     // Field TIM_TRK_1 is used to send update latency.
        private const int TIM_TRK_2_FID = 3903;     // Field TIM_TRK_2 is used to send post latency.
        private const int TIM_TRK_3_FID = 3904;     // Field TIM_TRK_3 is used to send post latency.

        private FieldList m_FieldList;              // field list 
        private FieldEntry m_FieldEntry;            // field entry 
        private UInt m_FidUIntValue;                // storage for UInt 
        private Int m_FidIntValue;                  // storage for Int
        private Real m_FidRealValue;                // storage for Real
        private Enum m_FidEnumValue;                // storage for Enum
        private Date m_FidDateValue;                // storage for Date
        private Time m_FidTimeValue;                // storage for Time
        private DateTime m_FidDateTimeValue;        // storage for DateTime
        private Float m_FidFloatValue;              // storage for Float
        private Double m_FidDoubleValue;            // storage for Double
        private Qos m_FidQosValue;                  // storage for QOS
        private State m_FidStateValue;              // storage for State
        private Buffer m_FidBufferValue;            // storage for Buffer 
        private IEnumType? m_EnumTypeInfo;           // EnumType dictionary information 
        private PostUserInfo m_PostUserInfo;        // post user information

        public MarketPriceDecoder(PostUserInfo postUserInfo)
        {
            m_FieldList = new FieldList();
            m_FieldEntry = new FieldEntry();
            m_FidUIntValue = new UInt();
            m_FidIntValue = new Int();
            m_FidRealValue = new Real();
            m_FidDoubleValue = new Double();
            m_FidTimeValue = new Time();
            m_FidDateTimeValue = new DateTime();
            m_FidFloatValue = new Float();
            m_FidQosValue = new Qos();
            m_FidStateValue = new State();
            m_FidBufferValue = new Buffer();
            m_PostUserInfo = postUserInfo;
            m_FidEnumValue = new Enum();
            m_FidDateValue = new Date();
        }

        /// <summary>
        /// Decode the update
        /// </summary>
        /// <param name="iter">the <see cref="DecodeIterator"/> instance used for decoding</param>
        /// <param name="msg">partially decoded update message</param>
        /// <param name="consumerThread">cnsumer thread information</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation</returns>
        public CodecReturnCode DecodeUpdate(DecodeIterator iter, Msg msg, ConsumerThreadInfo consumerThread)
        {
            CodecReturnCode codecReturnCode;
            IDictionaryEntry dictionaryEntry;

            long timeTracker = 0;
            long postTimeTracker = 0;
            long genMsgTimeTracker = 0;

            // decode field list
            if ((codecReturnCode = m_FieldList.Decode(iter, null)) == CodecReturnCode.SUCCESS)
            {
                if (codecReturnCode != CodecReturnCode.SUCCESS) return codecReturnCode;

                while ((codecReturnCode = m_FieldEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    // get dictionary entry
                    dictionaryEntry = consumerThread.Dictionary!.Entry(m_FieldEntry.FieldId);

                    if (dictionaryEntry == null)
                    {
                        Console.WriteLine($"Error: Decoded field ID {m_FieldEntry.FieldId} not present in dictionary.");
                        return CodecReturnCode.FAILURE;
                    }

                    // decode fid value
                    if ((codecReturnCode = DecodeFidValue(dictionaryEntry, iter, consumerThread)) < CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }

                    if (msg.MsgClass == MsgClasses.UPDATE && codecReturnCode != CodecReturnCode.BLANK_DATA)
                    {
                        if (m_FieldEntry.FieldId == TIM_TRK_1_FID)
                        {
                            timeTracker = m_FidUIntValue.ToLong();
                        }
                        if (m_FieldEntry.FieldId == TIM_TRK_2_FID)
                        {
                            postTimeTracker = m_FidUIntValue.ToLong();
                        }
                    }
                    else if (msg.MsgClass == MsgClasses.GENERIC && codecReturnCode != CodecReturnCode.BLANK_DATA)
                    {
                        if (m_FieldEntry.FieldId == TIM_TRK_3_FID)
                        {
                            genMsgTimeTracker = m_FidUIntValue.ToLong();
                        }
                    }
                }
            }
            else
            {
                return codecReturnCode;
            }

            if (timeTracker > 0)
            {
                UpdateLatencyStats(consumerThread, timeTracker, MsgClasses.UPDATE);
            }
            if (postTimeTracker > 0 && CheckPostUserInfo(msg))
            {
                UpdateLatencyStats(consumerThread, postTimeTracker, MsgClasses.POST);
            }
            if (genMsgTimeTracker > 0)
            {
                UpdateLatencyStats(consumerThread, genMsgTimeTracker, MsgClasses.GENERIC);
            }

            return CodecReturnCode.SUCCESS;
        }

        private void UpdateLatencyStats(ConsumerThreadInfo consumerThread, long timeTracker, int msgClass)
        {
            long currentTime = (long)GetTime.GetMicroseconds();
            long unitsPerMicro;

            unitsPerMicro = 1;

            switch (msgClass)
            {
                case MsgClasses.UPDATE:
                    consumerThread.TimeRecordSubmit(consumerThread.LatencyRecords, timeTracker, currentTime, unitsPerMicro);
                    break;
                case MsgClasses.GENERIC:
                    consumerThread.TimeRecordSubmit(consumerThread.GenMsgLatencyRecords, timeTracker, currentTime, unitsPerMicro);
                    break;
                case MsgClasses.POST:
                    consumerThread.TimeRecordSubmit(consumerThread.PostLatencyRecords, timeTracker, currentTime, unitsPerMicro);
                    break;
                default:
                    break;
            }
        }

        private CodecReturnCode DecodeFidValue(IDictionaryEntry dictionaryEntry, DecodeIterator iter, ConsumerThreadInfo consumerThread)
        {
            switch (dictionaryEntry.RwfType)
            {
                case DataTypes.INT:
                    return m_FidIntValue.Decode(iter);
                case DataTypes.UINT:
                    return m_FidUIntValue.Decode(iter);
                case DataTypes.FLOAT:
                    return m_FidFloatValue.Decode(iter);
                case DataTypes.DOUBLE:
                    return m_FidDoubleValue.Decode(iter);
                case DataTypes.REAL:
                    return m_FidRealValue.Decode(iter);
                case DataTypes.DATE:
                    return m_FidDateValue.Decode(iter);
                case DataTypes.TIME:
                    return m_FidTimeValue.Decode(iter);
                case DataTypes.DATETIME:
                    return m_FidDateTimeValue.Decode(iter);
                case DataTypes.QOS:
                    return m_FidQosValue.Decode(iter);
                case DataTypes.STATE:
                    return m_FidStateValue.Decode(iter);
                case DataTypes.ENUM:
                    {
                        CodecReturnCode codecReturnCode;
                        if ((codecReturnCode = m_FidEnumValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        {
                            return codecReturnCode;
                        }
                        if (codecReturnCode == CodecReturnCode.BLANK_DATA)
                            break;
                        m_EnumTypeInfo = consumerThread.Dictionary!.EntryEnumType(dictionaryEntry, m_FidEnumValue);
                        if (m_EnumTypeInfo != null)
                        {
                            m_FidBufferValue = m_EnumTypeInfo.Display;
                        }
                        break;
                    }
                case DataTypes.BUFFER:
                case DataTypes.ASCII_STRING:
                case DataTypes.UTF8_STRING:
                case DataTypes.RMTES_STRING:
                    return m_FidBufferValue.Decode(iter);
                default:
                    Console.WriteLine($"Error: Unhandled data type {DataTypes.ToString(dictionaryEntry.RwfType)}({dictionaryEntry.RwfType}) in field with ID {m_FieldEntry.FieldId}.\n");
                    return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.SUCCESS;
        }

        private bool CheckPostUserInfo(Msg msg)
        {
            // If post user info is present, make sure it matches our info.
            // Otherwise, assume any posted information present came from us anyway(return true).
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                case MsgClasses.UPDATE:
                    return !msg.CheckHasPostUserInfo() 
                        || msg.PostUserInfo.UserAddr == m_PostUserInfo.UserAddr 
                        && msg.PostUserInfo.UserId == m_PostUserInfo.UserId;
                default:
                    return true;
            }
        }
    }
}
