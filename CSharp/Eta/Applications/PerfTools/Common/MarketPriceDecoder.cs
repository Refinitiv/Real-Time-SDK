/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.PerfTools.Common
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

        private Func<DecodeIterator, CodecReturnCode>[] m_DecodeFunc = new Func<DecodeIterator, CodecReturnCode>[20];

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

            m_DecodeFunc[DataTypes.INT] = iter => m_FidIntValue.Decode(iter);
            m_DecodeFunc[DataTypes.UINT] = iter => m_FidUIntValue.Decode(iter);
            m_DecodeFunc[DataTypes.FLOAT] = iter => m_FidFloatValue.Decode(iter);
            m_DecodeFunc[DataTypes.DOUBLE] = iter => m_FidDoubleValue.Decode(iter);
            m_DecodeFunc[DataTypes.REAL] = iter => m_FidRealValue.Decode(iter);
            m_DecodeFunc[DataTypes.DATE] = iter => m_FidDateValue.Decode(iter);
            m_DecodeFunc[DataTypes.TIME] = iter => m_FidTimeValue.Decode(iter);
            m_DecodeFunc[DataTypes.DATETIME] = iter => m_FidDateTimeValue.Decode(iter);
            m_DecodeFunc[DataTypes.QOS] = iter => m_FidQosValue.Decode(iter);
            m_DecodeFunc[DataTypes.STATE] = iter => m_FidStateValue.Decode(iter);
            m_DecodeFunc[DataTypes.BUFFER] = iter => m_FidBufferValue.Decode(iter);
            m_DecodeFunc[DataTypes.ASCII_STRING] = iter => m_FidBufferValue.Decode(iter);
            m_DecodeFunc[DataTypes.UTF8_STRING] = iter => m_FidBufferValue.Decode(iter);
            m_DecodeFunc[DataTypes.RMTES_STRING] = iter => m_FidBufferValue.Decode(iter);
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

            switch (msgClass)
            {
                case MsgClasses.UPDATE:
                    consumerThread.TimeRecordSubmit(consumerThread.LatencyRecords, timeTracker, currentTime, 1);
                    break;
                case MsgClasses.GENERIC:
                    consumerThread.TimeRecordSubmit(consumerThread.GenMsgLatencyRecords, timeTracker, currentTime, 1);
                    break;
                case MsgClasses.POST:
                    consumerThread.TimeRecordSubmit(consumerThread.PostLatencyRecords, timeTracker, currentTime, 1);
                    break;
                default:
                    break;
            }
        }

        private CodecReturnCode DecodeFidValue(IDictionaryEntry dictionaryEntry, DecodeIterator iter, ConsumerThreadInfo consumerThread)
        {
            int type = dictionaryEntry.GetRwfType();
            if (type == DataTypes.ENUM)
            {
                CodecReturnCode codecReturnCode;
                if ((codecReturnCode = m_FidEnumValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                {
                    return codecReturnCode;
                }
                if (codecReturnCode != CodecReturnCode.BLANK_DATA)
                {
                    m_EnumTypeInfo = consumerThread!.Dictionary!.EntryEnumType(dictionaryEntry, m_FidEnumValue);
                    if (m_EnumTypeInfo != null)
                    {
                        m_FidBufferValue = m_EnumTypeInfo.Display;
                    }
                }

                return CodecReturnCode.SUCCESS;
            }
            else
            {
                return m_DecodeFunc[type](iter);
            }
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
