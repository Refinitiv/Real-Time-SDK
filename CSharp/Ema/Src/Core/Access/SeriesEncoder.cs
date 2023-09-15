/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal sealed class SeriesEncoder : Encoder
    {
        internal Eta.Codec.Series m_rsslSeries;
        internal Eta.Codec.SeriesEntry m_rsslSeriesEntry = new Eta.Codec.SeriesEntry();

        private bool m_containerInitialized = false;

        private int m_summaryDataType = DataType.DataTypes.NO_DATA;
        private bool m_containerTypeSet = false;
        private bool m_returnSummaryByteBuffer = false;
        internal ByteBuffer? m_summaryByteBuffer;

        internal SeriesEncoder(Series encoderOwner)
        {
            m_encoderOwner = encoderOwner;
            m_rsslSeries = encoderOwner.m_rsslSeries;
            EndEncodingEntry = EndEncodingEntryImpl;
        }

        private void InitEncode(int containerDataType)
        {
            if (!m_containerInitialized)
            {
                if (!m_containerTypeSet)
                {
                    m_rsslSeries.ContainerType = containerDataType;
                    m_containerTypeSet = true;
                }
                else if (m_rsslSeries.ContainerType != containerDataType)
                {
                    throw new OmmInvalidUsageException($"Attempt to set ContianerType {containerDataType} different than Summary's type {m_rsslSeries.ContainerType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }

                var ret = m_rsslSeries.EncodeInit(m_encodeIterator, 0, 0);
                while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
                {
                    ret = m_rsslSeries.EncodeComplete(m_encodeIterator, false);
                    ReallocateEncodeIteratorBuffer();
                    ret = m_rsslSeries.EncodeInit(m_encodeIterator, 0, 0);
                }
                if (ret < CodecReturnCode.SUCCESS)
                {
                    throw new OmmInvalidUsageException($"Failed to initialize Vector encoding, return code: {ret.GetAsString()}",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_containerInitialized = true;
            }
            else
            {
                throw new OmmInvalidUsageException($"Attempt to initialize Vector container when it is already initialized.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        private void AddEncodedEntry(Buffer load)
        {
            m_rsslSeriesEntry.Clear();
            m_rsslSeriesEntry.EncodedData = load;

            var ret = m_rsslSeriesEntry.Encode(m_encodeIterator);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslSeriesEntry.Encode(m_encodeIterator);
            }
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode Vector entry, return code: {ret.GetAsString()}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        private void StartEncodingEntry()
        {
            m_rsslSeriesEntry.Clear();

            var ret = m_rsslSeriesEntry.EncodeInit(m_encodeIterator, 0);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                m_rsslSeriesEntry.EncodeComplete(m_encodeIterator, false);
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslSeriesEntry.EncodeInit(m_encodeIterator, 0);
            }
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to intitalize encoding Vector entry, return code: {ret.GetAsString()}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        private void EndEncodingEntryImpl()
        {
            var ret = m_rsslSeriesEntry.EncodeComplete(m_encodeIterator, true);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslSeriesEntry.EncodeComplete(m_encodeIterator, true);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode SeriesEntry: {ret.GetAsString()}");
            }
        }

        public void TotalCountHint(int totalCountHint)
        {
            if (!m_containerInitialized)
            {
                m_rsslSeries.ApplyHasTotalCountHint();
                m_rsslSeries.TotalCountHint = totalCountHint;
            }
            else
            {
                throw new OmmInvalidUsageException("Invalid attempt to call TotalCountHint() when container is already initialized.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        public void SummaryData(ComplexType summary)
        {
            if (!m_containerInitialized)
            {
                Encoder? encoder = summary.Encoder;
                if (encoder != null && encoder.OwnsIterator())
                {
                    if (encoder.IsComplete)
                    {
                        m_rsslSeries.ApplyHasSummaryData();
                        CaptureSummaryByteBuffer(encoder);
                        m_rsslSeries.EncodedSummaryData.Data(m_summaryByteBuffer!.Flip());
                    }
                    else
                    {
                        throw new OmmInvalidUsageException("Attempt to set SummaryData() with a ComplexType while Complete() was not called on this ComplexType.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                }
                else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
                {
                    m_rsslSeries.ApplyHasSummaryData();
                    m_rsslSeries.EncodedSummaryData = encoder!.m_encoderOwner!.m_bodyBuffer;
                }
                else
                {
                    var dataType = (summary.DataType >= DataType.DataTypes.REQ_MSG && summary.DataType <= DataType.DataTypes.GENERIC_MSG)
                                    ? DataType.DataTypes.MSG
                                    : summary.DataType;
                    if (dataType == DataTypes.MSG)
                    {
                        ((Msg)summary!).EncodeComplete();
                        m_rsslSeries.ApplyHasSummaryData();
                        CaptureSummaryByteBuffer(encoder);
                        m_rsslSeries.EncodedSummaryData.Data(m_summaryByteBuffer!.Flip());
                    }
                    else
                    {
                        throw new OmmInvalidUsageException("Attempt to set an empty SummaryData while it is not supported.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }                 
                }

                m_summaryDataType = summary.DataType;
                m_rsslSeries.ContainerType = ConvertDataTypeToEta(summary.DataType);
                m_containerTypeSet = true;
            }
            else
            {
                throw new OmmInvalidUsageException("Invalid attempt to call SummaryData() when container is not empty.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        internal void CaptureSummaryByteBuffer(Encoder summaryEncoder)
        {
            m_summaryByteBuffer = summaryEncoder.m_iteratorByteBuffer;
            summaryEncoder.m_releaseIteratorByteBuffer = false;
            m_returnSummaryByteBuffer = true;
        }

        public void Add(ComplexType load)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            int dataType = ConvertDataTypeToEta(load.DataType);
            Encoder? encoder = load.Encoder;

            if (!m_containerInitialized)
            {
                AcquireEncodeIterator();
                InitEncode(dataType);
            }
            else
            {
                if (m_rsslSeries.ContainerType != dataType)
                {
                    throw new OmmInvalidUsageException($"Attempt to set ContianerType {dataType} different than Summary's type {m_rsslSeries.ContainerType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }
            }

            if (dataType != DataTypes.MSG)
            {
                if (encoder != null && encoder.m_encodeIterator != null && encoder.OwnsIterator())
                {
                    if (encoder.IsComplete)
                    {
                        AddEncodedEntry(encoder.m_encodeIterator!.Buffer());
                    }
                    else
                    {
                        throw new OmmInvalidUsageException("Attempt to add a ComplexType while Complete() was not called on this ComplexType.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                }
                else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
                {
                    AddEncodedEntry(encoder!.m_encoderOwner!.m_bodyBuffer!);
                }
                else
                {
                    PassEncIterator(encoder);
                    StartEncodingEntry();
                }
            }
            else
            {
                AddMessage((Msg)load);
            }        
        }

        private void AddMessage(Msg msg)
        {
            MsgEncoder encoder = (MsgEncoder)msg.Encoder!;
            if (encoder.m_encoded)
            {
                if (encoder.m_encodeIterator != null)
                {
                    encoder.EncodeComplete();
                    AddEncodedEntry(encoder.GetEncodedBuffer(false));
                }
                else
                {
                    StartEncodingEntry();
                    if (!encoder.EncodeComplete(m_encodeIterator!, out var error))
                        throw new OmmInvalidUsageException($"{error!}. Adding message that is not pre-encoded to container is not supported.");
                    EndEncodingEntryImpl();
                }
            }
            else if (msg.m_hasDecodedDataSet)
            {
                AddEncodedEntry(encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException("Adding empty message as container payload not supported.");
            }
        }

        public void Add()
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (!m_containerInitialized)
            {
                AcquireEncodeIterator();
                InitEncode(DataTypes.NO_DATA);
            }
            else
            {
                if (m_rsslSeries.ContainerType != DataTypes.NO_DATA)
                {
                    throw new OmmInvalidUsageException($"Attempt to set ContianerType {DataTypes.NO_DATA} different than Summary's type {m_rsslSeries.ContainerType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }
            }

            Buffer buffer = new Buffer();
            buffer.Clear();
            AddEncodedEntry(buffer);
        }

        public void Complete()
        {
            if (m_containerComplete)
            {
                return;
            }

            if (!m_containerInitialized)
            {
                AcquireEncodeIterator();
                InitEncode(m_summaryDataType);
            }

            var ret = m_rsslSeries.EncodeComplete(m_encodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to complete Vector encoding, return code: {ret.GetAsString()}");
            }

            if (!OwnsIterator() && m_iteratorOwner != null)
            {
                m_iteratorOwner.EndEncodingEntry!();
            }

            m_containerComplete = true;
        }

        public override void Clear()
        {
            m_containerInitialized = false;
            m_containerTypeSet = false;
            if (m_returnSummaryByteBuffer) m_etaPool.ReturnByteBuffer(m_summaryByteBuffer);
            m_returnSummaryByteBuffer = false;
            m_summaryDataType = DataType.DataTypes.NO_DATA;
            base.Clear();
            m_rsslSeries.Clear();
            m_rsslSeriesEntry.Clear();
        }

        ~SeriesEncoder()
        {
            if (m_returnSummaryByteBuffer) m_etaPool.ReturnByteBuffer(m_summaryByteBuffer);
        }
    }
}
