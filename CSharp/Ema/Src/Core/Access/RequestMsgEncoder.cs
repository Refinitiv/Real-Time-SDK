/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Collections.Generic;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class RequestMsgEncoder : MsgEncoder
    {
        private Eta.Codec.ElementList m_elementList = new Eta.Codec.ElementList();
        private Eta.Codec.ElementEntry m_elementEntry = new Eta.Codec.ElementEntry();
        private Array m_array = new Array();
        private ArrayEntry m_arrayEntry = new ArrayEntry();
        private DecodeIterator m_batchDecIter = new DecodeIterator();
        internal OmmArray m_batchArray = new OmmArray();
        internal Eta.Codec.Msg m_batchCheckMsg = new Eta.Codec.Msg();
        internal DecodeIterator m_batchMsgDecoder = new DecodeIterator();

        private const string VIEW_DATA_STRING = ":ViewData";
        private const string ITEM_LIST_STRING = ":ItemList";

        internal List<string>? BatchItemList { get; private set; }

        private Buffer m_TempBuffer = new Buffer();

        internal RequestMsgEncoder(RequestMsg requestMsg)
        {
            m_rsslMsg = requestMsg.m_rsslMsg;
            m_encoderOwner = requestMsg;
            EndEncodingEntry = ReqMsgEndEncodingAttributesOrPayload;
        }

        internal void Priority(int priorityClass, int priorityCount)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Priority when message is already initialized.");
            if (priorityClass < 0 || priorityClass > 255)
                throw new OmmInvalidUsageException("Passed priorityClass in priority is out of range. [0 - 255].");
            if (priorityCount < 0 || priorityCount > 65535)
                throw new OmmInvalidUsageException("Passed priorityCount in priority is out of range. [0 - 65535].");

            m_encoded = true;
            m_rsslMsg.ApplyHasPriority();
            m_rsslMsg.Priority.PriorityClass = priorityClass;
            m_rsslMsg.Priority.Count = priorityCount;
        }

        public void Qos(uint timeliness, uint rate)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Qos when message is already initialized.");

            m_encoded = true;
            m_rsslMsg.ApplyHasQos();
            Qos rsslQos = m_rsslMsg.Qos;
            rsslQos.Clear();
            m_rsslMsg.ApplyHasWorstQos();
            Qos rsslWQos = m_rsslMsg.WorstQos;
            rsslWQos.Clear();

            bool hasQosRange = false;

            switch (rate)
            {
                case RequestMsg.Rate.TICK_BY_TICK:
                    rsslQos.Rate(QosRates.TICK_BY_TICK);
                    rsslWQos.Rate(QosRates.TICK_BY_TICK);
                    break;
                case RequestMsg.Rate.JIT_CONFLATED:
                    rsslQos.Rate(QosRates.JIT_CONFLATED);
                    rsslWQos.Rate(QosRates.JIT_CONFLATED);
                    break;
                case RequestMsg.Rate.BEST_CONFLATED_RATE:
                    rsslQos.Rate(QosRates.TIME_CONFLATED);
                    rsslQos.RateInfo(1);

                    hasQosRange = true;
                    rsslWQos.Rate(QosRates.JIT_CONFLATED);
                    break;
                case RequestMsg.Rate.BEST_RATE:
                    rsslQos.Rate(QosRates.TICK_BY_TICK);

                    hasQosRange = true;
                    rsslWQos.Rate(QosRates.JIT_CONFLATED);
                    break;
                default:
                    if (rate <= 65535)
                    {
                        rsslQos.Rate(QosRates.TIME_CONFLATED);
                        rsslQos.RateInfo((int)rate);

                        rsslWQos.Rate(QosRates.TIME_CONFLATED);
                        rsslWQos.RateInfo((int)rate);
                    }
                    else
                    {
                        rsslQos.Rate(QosRates.JIT_CONFLATED);
                        rsslWQos.Rate(QosRates.JIT_CONFLATED);
                    }
                    break;
            }

            switch (timeliness)
            {
                case RequestMsg.Timeliness.REALTIME:
                    rsslQos.Timeliness(QosTimeliness.REALTIME);
                    rsslWQos.Timeliness(QosTimeliness.REALTIME);
                    break;
                case RequestMsg.Timeliness.BEST_DELAYED_TIMELINESS:
                    rsslQos.Timeliness(QosTimeliness.DELAYED);
                    rsslQos.TimeInfo(1);

                    hasQosRange = true;
                    rsslWQos.Timeliness(QosTimeliness.DELAYED);
                    rsslWQos.TimeInfo(65535);
                    break;
                case RequestMsg.Timeliness.BEST_TIMELINESS:
                    rsslQos.Timeliness(QosTimeliness.REALTIME);

                    hasQosRange = true;
                    rsslWQos.Timeliness(QosTimeliness.DELAYED);
                    rsslWQos.TimeInfo(65535);
                    break;
                default:
                    if (timeliness <= 65535)
                    {
                        rsslQos.Timeliness(QosTimeliness.DELAYED);
                        rsslQos.TimeInfo((int)timeliness);

                        rsslWQos.Timeliness(QosTimeliness.DELAYED);
                        rsslWQos.TimeInfo((int)timeliness);
                    }
                    else
                    {
                        rsslQos.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
                        rsslWQos.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
                    }
                    break;
            }

            if (hasQosRange)
            {
                rsslQos.IsDynamic = true;
                rsslWQos.IsDynamic = true;
            }
            else
            {
                m_rsslMsg.Flags &= ~RequestMsgFlags.HAS_WORST_QOS;
            }
        }

        public void InitialImage(bool initialImage)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Initial image when message is already initialized.");

            m_encoded = true;
            if (initialImage)
            {
                m_rsslMsg.Flags &= ~RequestMsgFlags.NO_REFRESH;
            }
            else
            {
                m_rsslMsg.Flags |= RequestMsgFlags.NO_REFRESH;
            }
        }

        public void InterestAfterRefresh(bool interestAfterRefresh)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Interest after refresh when message is already initialized.");

            m_encoded = true;
            if (interestAfterRefresh)
            {
                m_rsslMsg.Flags |= RequestMsgFlags.STREAMING;
            }
            else
            {
                m_rsslMsg.Flags &= ~RequestMsgFlags.STREAMING;
            }
        }

        public void Pause(bool pause)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set pause when message is already initialized.");

            m_encoded = true;
            if (pause)
            {
                m_rsslMsg.Flags |= RequestMsgFlags.PAUSE;
            }
            else
            {
                m_rsslMsg.Flags &= ~RequestMsgFlags.PAUSE;
            }
        }

        public void ConflatedInUpdates(bool conflatedInUpdates)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Conflated in Updates when message is already initialized.");

            m_encoded = true;
            if (conflatedInUpdates)
            {
                m_rsslMsg.Flags |= RequestMsgFlags.CONF_INFO_IN_UPDATES;
            }
            else
            {
                m_rsslMsg.Flags &= ~RequestMsgFlags.CONF_INFO_IN_UPDATES;
            }
        }

        public void PrivateStream(bool privateStream)
        {
            if (m_msgEncodeInitialized)
                throw new OmmInvalidUsageException("Invalid attempt to set Private Stream when message is already initialized.");

            m_encoded = true;
            if (privateStream)
            {
                m_rsslMsg.Flags |= RequestMsgFlags.PRIVATE_STREAM;
            }
            else
            {
                m_rsslMsg.Flags &= ~RequestMsgFlags.PRIVATE_STREAM;
            }
        }

        public void RequestMsgPayload(ComplexType payload)
        {
            if (payload.DataType != DataType.DataTypes.ELEMENT_LIST)
            {
                Payload(payload);
            }
            else
            {
                Payload(payload);
                if (m_preencoded)
                {
                    CheckBatchView(m_rsslMsg.EncodedDataBody);
                }
            }
        }

        public override void Clear()
        {
            base.Clear();
            m_batchArray.ClearInt();
            BatchItemList?.Clear();
        }

        internal void ReqMsgEndEncodingAttributesOrPayload()
        {
            EndEncodingAttributesOrPayload();
            if (m_msgPayloadEncodeStarted && m_msgEncodeCompleted && m_rsslMsg.ContainerType == DataTypes.ELEMENT_LIST)
            {
                var bodyBuffer = m_encodeIterator!.Buffer();
                m_batchMsgDecoder.Clear();
                m_batchCheckMsg.Clear();
                m_batchMsgDecoder.SetBufferAndRWFVersion(bodyBuffer, Codec.MajorVersion(), Codec.MinorVersion());
                CodecReturnCode ret;
                if ((ret = m_batchCheckMsg.Decode(m_batchMsgDecoder)) < CodecReturnCode.SUCCESS)
                    throw new OmmInvalidUsageException($"Error while decoding message to check for Batch/View: {ret.GetAsString()}");
                CheckBatchView(m_batchCheckMsg.EncodedDataBody);
            }
        }

        internal void CheckBatchView(Buffer buffer)
        {
            m_elementList.Clear();
            m_elementEntry.Clear();
            m_batchDecIter.Clear();
            m_batchArray.ClearInt();

            var ret = m_batchDecIter.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());
            if (ret < CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException($"RequestMsgEncoder.CheckBatchView: Failed to set iterator buffer while decoding RequestMsg body: {ret.GetAsString()}");

            ret = m_elementList.Decode(m_batchDecIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                throw new OmmInvalidUsageException($"RequestMsgEncoder.CheckBatchView: Failed to decode ElementList in RequestMsg payload: {ret.GetAsString()}");

            while (true)
            {
                ret = m_elementEntry.Decode(m_batchDecIter);

                switch (ret)
                {
                    case CodecReturnCode.END_OF_CONTAINER:
                        return;

                    case CodecReturnCode.SUCCESS:
                        if (m_elementEntry.DataType == Eta.Codec.DataTypes.ARRAY)
                        {
                            if (m_elementEntry.Name.ToString().Equals(VIEW_DATA_STRING))
                            {
                                m_rsslMsg.Flags |= RequestMsgFlags.HAS_VIEW;
                            }
                            else if (m_elementEntry.Name.ToString().Equals(ITEM_LIST_STRING))
                            {
                                m_rsslMsg.Flags |= RequestMsgFlags.HAS_BATCH;
                                m_batchArray.SetRsslData(m_batchDecIter.MajorVersion(),
                                    m_batchDecIter.MinorVersion(),
                                    m_elementEntry.EncodedData,
                                    null);

                                if (GetBatchItemList(m_batchDecIter) < CodecReturnCode.SUCCESS)
                                {
                                    throw new OmmInvalidUsageException($"RequestMsgEncoder.CheckBatchView: Failed to decode ArrayEntry in RequestMsg payload: {ret.GetAsString()}");
                                }
                                else if (BatchItemList?.Count == 0)
                                {
                                    throw new OmmInvalidUsageException($"RequestMsgEncoder.CheckBatchView: Batch request item list is blank.");
                                }
                            }
                        }
                        break;

                    case CodecReturnCode.INCOMPLETE_DATA:
                    case CodecReturnCode.UNSUPPORTED_DATA_TYPE:
                    default:
                        throw new OmmInvalidUsageException($"RequestMsgEncoder.CheckBatchView: Failed to decode ElementEntry in RequestMsg payload: {ret.GetAsString()}");
                }
            }
        }

        private CodecReturnCode GetBatchItemList(DecodeIterator decodeIterator)
        {
            CodecReturnCode ret;

            if (BatchItemList is null)
                BatchItemList = new();
            else
                BatchItemList.Clear();

            m_array.Clear();
            if ((ret = m_array.Decode(decodeIterator)) >= CodecReturnCode.SUCCESS)
            {
                if (m_array.PrimitiveType == DataTypes.ASCII_STRING)
                {
                    m_arrayEntry.Clear();
                    m_TempBuffer.Clear();
                    while ((ret = m_arrayEntry.Decode(decodeIterator)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        if (m_TempBuffer.Decode(decodeIterator) == CodecReturnCode.SUCCESS) // Skip blank data
                        {
                            BatchItemList.Add(m_TempBuffer.ToString());
                        }
                    }
                }
            }

            return ret;
        }
    }
}
