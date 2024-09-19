/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.Codec;
using LSEG.Eta.PerfTools.Common;

using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Eta.PerfTools.ProvPerf
{
    /// <summary>
    /// This is the market price decoder for the ProvPerf application
    /// </summary>
    public class IProvMarketPriceDecoder
    {
        private const int TIM_TRK_3_FID = 3904; // Field TIM_TRK_3 is used to send generic message latency.

        private FieldList _fList;
        private FieldEntry _fEntry;
        private UInt _fidUIntValue; 
        private Int _fidIntValue;
        private Real _fidRealValue;
        private Enum _fidEnumValue;
        private Date _fidDateValue;
        private Time _fidTimeValue;
        private DateTime _fidDateTimeValue;
        private Float _fidFloatValue;
        private Double _fidDoubleValue;
        private Qos _fidQosValue;
        private State _fidStateValue;
        private Buffer _fidBufferValue;
        private IEnumType? _enumTypeInfo;

        /// <summary>
        /// Instantiates a new market price decoder
        /// </summary>
        public IProvMarketPriceDecoder()
        {
            _fList = new FieldList();
            _fEntry = new FieldEntry();
            _fidUIntValue = new UInt();
            _fidIntValue = new Int();
            _fidRealValue = new Real();
            _fidEnumValue = new Enum();
            _fidDateValue = new Date();
            _fidTimeValue = new Time();
            _fidDateTimeValue = new DateTime();
            _fidFloatValue = new Float();
            _fidDoubleValue = new Double();
            _fidQosValue = new Qos();
            _fidStateValue = new State();
            _fidBufferValue = new Buffer();
        }

        /// <summary>
        /// Update the latency statistics
        /// </summary>
        /// <param name="provThreadInfo">provider thread information</param>
        /// <param name="timeTracker">start time</param>
        /// <param name="msgClass">the message class</param>
        private void UpdateLatencyStats(ProviderThreadInfo provThreadInfo, long timeTracker, int msgClass)
        {
            provThreadInfo.TimeRecordSubmit(provThreadInfo.GenMsgLatencyRecords, timeTracker, (long)GetTime.GetMicroseconds(), 1);
        }

        /// <summary>
        /// Decode the update
        /// </summary>
        /// <param name="iter"><see cref="DecodeIterator"/> instance used to decode current message</param>
        /// <param name="msg">partially decoded update message</param>
        /// <param name="provThreadInfo">provider thread information</param>
        /// <returns><see cref="CodecReturnCode"/> indicating the status of the operation</returns>
        public CodecReturnCode DecodeUpdate(DecodeIterator iter, Msg msg, ProviderThreadInfo provThreadInfo)
        {
            CodecReturnCode codecReturnCode;
            IDictionaryEntry dictionaryEntry;

            long timeTracker = 0;

            // decode field list
            if ((codecReturnCode = _fList.Decode(iter, null)) == CodecReturnCode.SUCCESS)
            {
                if (codecReturnCode != CodecReturnCode.SUCCESS)
                    return codecReturnCode;

                while ((codecReturnCode = _fEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    // get dictionary entry
                    dictionaryEntry = provThreadInfo.Dictionary!.Entry(_fEntry.FieldId);

                    if (dictionaryEntry == null)
                    {
                        Console.WriteLine($"Error: Decoded field ID {_fEntry.FieldId} not present in dictionary.\n");
                        return CodecReturnCode.FAILURE;
                    }

                    // decode fid value
                    if ((codecReturnCode = DecodeFidValue(dictionaryEntry, iter, provThreadInfo)) < CodecReturnCode.SUCCESS)
                    {
                        return codecReturnCode;
                    }

                    if (msg.MsgClass == MsgClasses.GENERIC && codecReturnCode != CodecReturnCode.BLANK_DATA)
                    {
                        if (_fEntry.FieldId == TIM_TRK_3_FID)
                            timeTracker = _fidUIntValue.ToLong();
                    }
                }
            }
            else
            {
                return codecReturnCode;
            }

            if (timeTracker > 0)
                UpdateLatencyStats(provThreadInfo, timeTracker, MsgClasses.GENERIC);

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Decode FID value
        /// </summary>
        /// <param name="dictionaryEntry">dictionary entry that corresponds to the decoded FID</param>
        /// <param name="iter"><see cref="DecodeIterator"/> used to decode current FID</param>
        /// <param name="provThreadInfo">provider thread information</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation</returns>
        private CodecReturnCode DecodeFidValue(IDictionaryEntry dictionaryEntry, DecodeIterator iter, ProviderThreadInfo provThreadInfo)
        {
            CodecReturnCode codecReturnCode;

            int dataType = dictionaryEntry.GetRwfType();
            switch (dataType)
            {
                case DataTypes.INT:
                    if ((codecReturnCode = _fidIntValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.UINT:
                    if ((codecReturnCode = _fidUIntValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.FLOAT:
                    if ((codecReturnCode = _fidFloatValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.DOUBLE:
                    if ((codecReturnCode = _fidDoubleValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.REAL:
                    if ((codecReturnCode = _fidRealValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.DATE:
                    if ((codecReturnCode = _fidDateValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.TIME:
                    if ((codecReturnCode = _fidTimeValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.DATETIME:
                    if ((codecReturnCode = _fidDateTimeValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.QOS:
                    if ((codecReturnCode = _fidQosValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.STATE:
                    if ((codecReturnCode = _fidStateValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                case DataTypes.ENUM:
                    {
                        if ((codecReturnCode = _fidEnumValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                            return codecReturnCode;

                        if (codecReturnCode == CodecReturnCode.BLANK_DATA)
                            break;

                        _enumTypeInfo = provThreadInfo.Dictionary!.EntryEnumType(dictionaryEntry, _fidEnumValue);
                        if (_enumTypeInfo != null)
                            _fidBufferValue = _enumTypeInfo.Display;
                        break;
                    }
                case DataTypes.BUFFER:
                case DataTypes.ASCII_STRING:
                case DataTypes.UTF8_STRING:
                case DataTypes.RMTES_STRING:
                    if ((codecReturnCode = _fidBufferValue.Decode(iter)) < CodecReturnCode.SUCCESS)
                        return codecReturnCode;
                    break;
                default:
                    Console.Write($"Error: Unhandled data type {DataTypes.ToString(dataType)} ({dataType}) in field with ID {_fEntry.FieldId}.\n");
                    return CodecReturnCode.FAILURE;
            }

            return codecReturnCode;
        }
    }
}
