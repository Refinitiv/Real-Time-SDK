/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;
using LSEG.Ema.Access;
using System;
using System.Diagnostics;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.PerfTools.ConsPerf
{
    internal class MarketPriceDecoder
    {
        private const int TIM_TRK_1_FID = 3902; /* Field TIM_TRK_1 is used to send update latency. */
        private const int TIM_TRK_2_FID = 3903; /* Field TIM_TRK_2 is used to send post latency. */
        private const int TIM_TRK_3_FID = 3904; /* Field TIM_TRK_3 is used to send post latency. */

        private FieldEntry? _fEntry; /* field entry */
        private OmmUInt? _fidUIntValue; /* storage for UInt */
        private OmmInt? _fidIntValue; /* storage for Int */
        private OmmReal? _fidRealValue; /* storage for Real */
        private OmmEnum? _fidEnumValue; /* storage for Enum */
        private OmmDate? _fidDateValue; /* storage for Date */
        private OmmTime? _fidTimeValue; /* storage for Time */
        private OmmDateTime? _fidDateTimeValue; /* storage for DateTime */
        private OmmFloat? _fidFloatValue; /* storage for Float */
        private OmmDouble? _fidDoubleValue; /* storage for Double */
        private OmmQos? _fidQosValue; /* storage for QOS */
        private OmmState? _fidStateValue; /* storage for State */
        private OmmBuffer? _fidBufferValue; /* storage for Buffer */
        private OmmRmtes? _fidRmtesValue; /* storage for Buffer */
        private OmmUtf8? _fidUtf8Value; /* storage for Buffer */
        private OmmAscii? _fidAsciiValue; /* storage for Buffer */       
        private PostUserInfo? _postUserInfo; /* post user information */

        private Action<FieldEntry>?[] m_handleFieldEntryActions;

        /**
         * Instantiates a new market price decoder.
         *
         * @param postUserInfo the post user info
         */
        public MarketPriceDecoder(PostUserInfo postUserInfo)
        {
            _postUserInfo = postUserInfo;
            m_handleFieldEntryActions = new Action<FieldEntry>?[20]
            {
                null,
                null,
                null,
                entry => _fidIntValue = entry.OmmIntValue(),
                entry => _fidUIntValue = entry.OmmUIntValue(),
                entry => _fidFloatValue = entry.OmmFloatValue(),
                entry => _fidDoubleValue = entry.OmmDoubleValue(),
                null,
                entry => _fidRealValue = entry.OmmRealValue(),
                entry => _fidDateValue = entry.OmmDateValue(),
                entry => _fidTimeValue = entry.OmmTimeValue(),
                entry => _fidDateTimeValue = entry.OmmDateTimeValue(),
                entry => _fidQosValue = entry.OmmQosValue(),
                entry => _fidStateValue = entry.OmmStateValue(),
                entry => _fidEnumValue = entry.OmmEnumValue(),
                null,
                entry => _fidBufferValue = entry.OmmBufferValue(),
                entry => _fidAsciiValue = entry.OmmAsciiValue(),
                entry => _fidUtf8Value = entry.OmmUtf8Value(),
                entry => _fidRmtesValue = entry.OmmRmtesValue()
            };
        }

        /**
         *  Decode the update.
         *
         * @param msg the EMA msg
         * @param fieldList the field list
         * @param _consThreadInfo the cons thread info
         * @param downcastDecoding the downcast decoding
         * @return true, if successful
         */
        public bool DecodeResponse(Msg msg, FieldList fieldList, ConsumerThreadInfo _consThreadInfo, bool downcastDecoding)
        {
            long timeTracker = 0;
            long postTimeTracker = 0;
            long genMsgTimeTracker = 0;

            try
            {
                /* decode field list */
                var enumerator = fieldList.GetEnumerator();

                if (!downcastDecoding)
                {
                    while (enumerator.MoveNext())
                    {
                        _fEntry = enumerator.Current;
                        if (Data.DataCode.NO_CODE == _fEntry.Code)
                        {
                            int loadType = _fEntry.LoadType;
                            if (loadType < m_handleFieldEntryActions.Length)
                            {
                                m_handleFieldEntryActions[_fEntry.LoadType]!(_fEntry);
                            }
                            else
                            {
                                Console.Error.WriteLine($"Error: Unhandled data type {DataType.AsString(_fEntry.LoadType)} in field with ID {_fEntry.FieldId}.\n");
                                return false;
                            }
                            
                            if (msg.DataType == DataTypes.UPDATE_MSG)
                            {
                                if (_fEntry.FieldId == TIM_TRK_1_FID)
                                    timeTracker = (long)_fidUIntValue!.Value;
                                if (_fEntry.FieldId == TIM_TRK_2_FID)
                                    postTimeTracker = (long)_fidUIntValue!.Value;
                            }
                            else if (msg.DataType == DataTypes.GENERIC_MSG)
                            {
                                if (_fEntry.FieldId == TIM_TRK_3_FID)
                                    genMsgTimeTracker = (long)_fidUIntValue!.Value;
                            }
                        }
                    }
                }
                else
                {
                    while (enumerator.MoveNext())
                    {
                        _fEntry = enumerator.Current;
                        if (Data.DataCode.NO_CODE == _fEntry.Code)
                        {
                            switch (_fEntry.LoadType)
                            {
                                case DataTypes.INT:
                                    _fidIntValue = (OmmInt)_fEntry.Load!;
                                    break;
                                case DataTypes.UINT:
                                    _fidUIntValue = (OmmUInt)_fEntry.Load!; ;
                                    break;
                                case DataTypes.FLOAT:
                                    _fidFloatValue = (OmmFloat)_fEntry.Load!;
                                    break;
                                case DataTypes.DOUBLE:
                                    _fidDoubleValue = (OmmDouble)_fEntry.Load!;
                                    break;
                                case DataTypes.REAL:
                                    _fidRealValue = (OmmReal)_fEntry.Load!;
                                    break;
                                case DataTypes.DATE:
                                    _fidDateValue = (OmmDate)_fEntry.Load!;
                                    break;
                                case DataTypes.TIME:
                                    _fidTimeValue = (OmmTime)_fEntry.Load!;
                                    break;
                                case DataTypes.DATETIME:
                                    _fidDateTimeValue = (OmmDateTime)_fEntry.Load!;
                                    break;
                                case DataTypes.QOS:
                                    _fidQosValue = (OmmQos)_fEntry.Load!;
                                    break;
                                case DataTypes.STATE:
                                    _fidStateValue = (OmmState)_fEntry.Load!;
                                    break;
                                case DataTypes.ENUM:
                                    _fidEnumValue = (OmmEnum)_fEntry.Load!;
                                    break;
                                case DataTypes.BUFFER:
                                    _fidBufferValue = (OmmBuffer)_fEntry.Load!;
                                    break;
                                case DataTypes.ASCII:
                                    _fidAsciiValue = (OmmAscii)_fEntry.Load!;
                                    break;
                                case DataTypes.UTF8:
                                    _fidUtf8Value = (OmmUtf8)_fEntry.Load!;
                                    break;
                                case DataTypes.RMTES:
                                    _fidRmtesValue = (OmmRmtes)_fEntry.Load!;
                                    break;
                                default:
                                    Console.Error.WriteLine($"Error: Unhandled data type {DataType.AsString(_fEntry.LoadType)} in field with ID {_fEntry.FieldId}.\n");
                                    return false;
                            }

                            if (msg.DataType == DataTypes.UPDATE_MSG)
                            {
                                if (_fEntry.FieldId == TIM_TRK_1_FID)
                                    timeTracker = (long)_fidUIntValue!.Value;
                                if (_fEntry.FieldId == TIM_TRK_2_FID)
                                    postTimeTracker = (long)_fidUIntValue!.Value;
                            }
                            else if (msg.DataType == DataTypes.GENERIC_MSG)
                            {
                                if (_fEntry.FieldId == TIM_TRK_3_FID)
                                    genMsgTimeTracker = (long)_fidUIntValue!.Value;
                            }
                        }
                    }
                }

                enumerator.Dispose();
            }
            catch (OmmException excp)
            {
                Console.Error.WriteLine(excp);
            }

            if (timeTracker > 0)
                _consThreadInfo.TimeRecordSubmit(_consThreadInfo.LatencyRecords, timeTracker, (long)GetTime.GetMicroseconds(), 1);

            if (postTimeTracker > 0 && CheckPostUserInfo(msg))
                _consThreadInfo.TimeRecordSubmit(_consThreadInfo.PostLatencyRecords, postTimeTracker, (long)GetTime.GetMicroseconds(), 1);
            if (genMsgTimeTracker > 0)
                _consThreadInfo.TimeRecordSubmit(_consThreadInfo.GenMsgLatencyRecords, genMsgTimeTracker, (long)GetTime.GetMicroseconds(), 1);

            return true;
        }

        private bool CheckPostUserInfo(Msg msg)
        {
            /* If post user info is present, make sure it matches our info.
             * Otherwise, assume any posted information present came from us anyway(return true). */
            switch (msg.DataType)
            {
                case DataTypes.REFRESH_MSG:
                    RefreshMsg refreshMsg = (RefreshMsg)msg;
                    return !refreshMsg.HasPublisherId ||
                     refreshMsg.PublisherIdUserAddress() == _postUserInfo!.UserAddr &&
                     refreshMsg.PublisherIdUserId() == _postUserInfo.UserId;
                case DataTypes.UPDATE_MSG:
                    UpdateMsg updateMsg = (UpdateMsg)msg;
                    return !updateMsg.HasPublisherId || updateMsg.PublisherIdUserAddress() == _postUserInfo!.UserAddr && updateMsg.PublisherIdUserId() == _postUserInfo.UserId;
                default:
                    return true;
            }
        }
    }
}
