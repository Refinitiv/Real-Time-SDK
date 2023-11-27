/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal sealed class VectorEncoder : Encoder
    {
        internal Eta.Codec.Vector m_rsslVector;
        internal Eta.Codec.VectorEntry m_rsslVectorEntry = new Eta.Codec.VectorEntry();

        private bool m_containerInitialized = false;

        private int m_summaryDataType = DataType.DataTypes.NO_DATA;
        private bool m_containerTypeSet = false;
        internal bool m_returnSummaryByteBuffer = false;
        internal ByteBuffer? m_summaryByteBuffer;

        Buffer buffer = new Buffer();

        internal VectorEncoder(Vector encoderOwner)
        {
            m_encoderOwner = encoderOwner;
            m_rsslVector = encoderOwner.m_rsslVector;
            EndEncodingEntry = EndEncodingEntryImpl;
        }

        private void InitEncode(int containerDataType)
        {
            if (!m_containerInitialized)
            {
                if (!m_containerTypeSet)
                {
                    m_rsslVector.ContainerType = containerDataType;
                    m_containerTypeSet = true;
                }
                else if (m_rsslVector.ContainerType != containerDataType)
                {
                    throw new OmmInvalidUsageException($"Attempt to set ContianerType {containerDataType} different than Summary's type {m_rsslVector.ContainerType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }

                var ret = m_rsslVector.EncodeInit(m_encodeIterator, 0, 0);
                while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
                {
                    ret = m_rsslVector.EncodeComplete(m_encodeIterator, false);
                    ReallocateEncodeIteratorBuffer();
                    ret = m_rsslVector.EncodeInit(m_encodeIterator, 0, 0);
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

        private void AddEncodedEntry(uint position, int action, Buffer load, EmaBuffer? permission)
        {
            m_rsslVectorEntry.Clear();
            m_rsslVectorEntry.EncodedData = load;
            m_rsslVectorEntry.Index = position;
            m_rsslVectorEntry.Action = (VectorEntryActions)action;

            if (permission != null && permission.Length > 0)
            {
                m_rsslVectorEntry.ApplyHasPermData();
                m_rsslVectorEntry.PermData.Data(new ByteBuffer(permission.AsByteArray()).Flip());
            }

            var ret = m_rsslVectorEntry.Encode(m_encodeIterator);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslVectorEntry.Encode(m_encodeIterator);
            }
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode Vector entry, return code: {ret.GetAsString()}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        private void StartEncodingEntry(uint position, int action, EmaBuffer? permission)
        {
            m_rsslVectorEntry.Clear();
            m_rsslVectorEntry.Flags = VectorEntryFlags.NONE;
            m_rsslVectorEntry.Index = position;
            m_rsslVectorEntry.Action = (VectorEntryActions)action;

            if (permission != null && permission.Length > 0)
            {
                m_rsslVectorEntry.ApplyHasPermData();
                m_rsslVectorEntry.PermData.Data(new ByteBuffer(permission.AsByteArray()).Flip());
            }

            var ret = m_rsslVectorEntry.EncodeInit(m_encodeIterator, 0);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                m_rsslVectorEntry.EncodeComplete(m_encodeIterator, false);
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslVectorEntry.EncodeInit(m_encodeIterator, 0);
            }
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to intitalize encoding Vector entry, return code: {ret.GetAsString()}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        private void EndEncodingEntryImpl()
        {
            var ret = m_rsslVectorEntry.EncodeComplete(m_encodeIterator, true);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslVectorEntry.EncodeComplete(m_encodeIterator, true);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode VectorEntry: {ret.GetAsString()}");
            }
        }

        public void TotalCountHint(int totalCountHint)
        {
            if (!m_containerInitialized)
            {
                m_rsslVector.ApplyHasTotalCountHint();
                m_rsslVector.TotalCountHint = totalCountHint;
            }
            else
            {
                throw new OmmInvalidUsageException("Invalid attempt to call TotalCountHint() when container is already initialized.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        public void Sortable(bool sortable)
        {
            if (!m_containerInitialized)
            {
                if (sortable)
                {
                    m_rsslVector.Flags |= VectorFlags.SUPPORTS_SORTING;
                } 
                else
                {
                    m_rsslVector.Flags &= ~VectorFlags.SUPPORTS_SORTING;
                }
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
                if (encoder != null && encoder.m_encodeIterator != null && encoder.OwnsIterator())
                {
                    if (!encoder.IsComplete)
                        throw new OmmInvalidUsageException("Attempt to set SummaryData() with a ComplexType while Complete() was not called on this ComplexType.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    m_rsslVector.ApplyHasSummaryData();
                    CaptureSummaryByteBuffer(encoder);
                    m_rsslVector.EncodedSummaryData.Data(m_summaryByteBuffer!.Flip());
                }
                else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
                {
                    m_rsslVector.ApplyHasSummaryData();
                    m_rsslVector.EncodedSummaryData = encoder!.m_encoderOwner!.m_bodyBuffer;
                }
                else
                {
                    var dataType = (summary.m_dataType >= DataType.DataTypes.REQ_MSG && summary.m_dataType <= DataType.DataTypes.GENERIC_MSG)
                                    ? DataType.DataTypes.MSG
                                    : summary.m_dataType;
                    if (dataType == DataTypes.MSG)
                    {
                        ((Msg)summary!).EncodeComplete();
                        m_rsslVector.ApplyHasSummaryData(); 
                        CaptureSummaryByteBuffer(encoder);
                        m_rsslVector.EncodedSummaryData.Data(m_summaryByteBuffer!.Flip());
                    }
                    else
                        throw new OmmInvalidUsageException("Attempt to set an empty SummaryData while it is not supported.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }

                m_summaryDataType = summary.m_dataType;
                m_rsslVector.ContainerType = ConvertDataTypeToEta(summary.m_dataType);
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

        public void Add(uint position, int action, ComplexType load, EmaBuffer? permission)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            int dataType = ConvertDataTypeToEta(load.m_dataType);
            Encoder? encoder = load.Encoder;

            if (!m_containerInitialized)
            {
                AcquireEncodeIterator();
                InitEncode(dataType);
            } 
            else
            {
                if (m_rsslVector.ContainerType != dataType)
                {
                    throw new OmmInvalidUsageException($"Attempt to set ContianerType {dataType} different than Summary's type {m_rsslVector.ContainerType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }
            }

            if (action == VectorAction.DELETE || action == VectorAction.CLEAR)
            {
                Buffer buffer = new Buffer();
                buffer.Clear();
                AddEncodedEntry(position, action, buffer, permission);
                return;
            }

            if (dataType != DataType.DataTypes.MSG)
            {
                if (encoder != null && encoder.m_encodeIterator != null && encoder.OwnsIterator())
                {
                    if (!encoder.IsComplete)
                        throw new OmmInvalidUsageException("Attempt to add a ComplexType while Complete() was not called on this ComplexType.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    AddEncodedEntry(position, action, encoder.GetEncodedBuffer(false), permission);
                }
                else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
                {
                    AddEncodedEntry(position, action, encoder!.m_encoderOwner!.m_bodyBuffer!, permission);
                }
                else
                {
                    PassEncIterator(encoder);
                    StartEncodingEntry(position, action, permission);
                }
            }
            else
            {
                AddMessage(position, action, (Msg)load, permission);
            }
            
        }

        public void Add(uint position, int action, EmaBuffer? permission)
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
                if (m_rsslVector.ContainerType != DataTypes.NO_DATA)
                {
                    throw new OmmInvalidUsageException($"Attempt to set ContianerType {DataTypes.NO_DATA} different than Summary's type {m_rsslVector.ContainerType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }
            }

            buffer.Clear();
            AddEncodedEntry(position, action, buffer, permission);
        }

        private void AddMessage(uint position, int action, Msg msg, EmaBuffer? permission)
        {
            MsgEncoder encoder = msg.m_msgEncoder!;
            if (encoder.m_encoded)
            {
                if (encoder.m_encodeIterator != null)
                {
                    encoder.EncodeComplete();
                    AddEncodedEntry(position, action, encoder.GetEncodedBuffer(false), permission);
                }
                else
                {
                    StartEncodingEntry(position, action, permission);
                    if (!encoder.EncodeComplete(m_encodeIterator!, out var error))
                        throw new OmmInvalidUsageException($"{error!}. Adding message that is not pre-encoded to container is not supported.");
                    EndEncodingEntryImpl();
                }
            }
            else if (msg.m_hasDecodedDataSet)
            {
                AddEncodedEntry(position, action, encoder!.m_encoderOwner!.m_bodyBuffer!, permission);
            }
            else
            {
                throw new OmmInvalidUsageException("Adding empty message as container payload not supported.");
            }
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

            var ret = m_rsslVector.EncodeComplete(m_encodeIterator, true);
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
            base.Clear();
            if (m_returnSummaryByteBuffer)
            {
                EtaObjectGlobalPool.Instance.ReturnByteBuffer(m_summaryByteBuffer);
            }
            m_returnSummaryByteBuffer = false;
            m_containerInitialized = false;
            m_rsslVector.Clear();
            m_rsslVectorEntry.Clear();
            m_containerTypeSet = false;
        }

        ~VectorEncoder()
        {
            if (m_releaseIteratorByteBuffer) m_etaPool.ReturnByteBuffer(m_summaryByteBuffer);
        }
    }
}
