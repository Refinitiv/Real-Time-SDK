/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access
{
    internal class MapEncoder : Encoder
    {
        internal Eta.Codec.Map m_rsslMap;
        internal Eta.Codec.MapEntry m_rsslMapEntry = new Eta.Codec.MapEntry();

        private bool m_containerInitialized = false;

        private int m_emaDataType = DataType.DataTypes.NO_DATA;
        private int m_emaKeyType;
        private bool m_containerTypeSet = false;
        private bool m_keyTypeSet = false;
        private bool m_returnSummaryByteBuffer = false;
        internal ByteBuffer? m_summaryByteBuffer;

        internal MapEncoder(Map encoderOwner)
        {
            m_encoderOwner = encoderOwner;
            m_rsslMap = encoderOwner.m_rsslMap;
            EndEncodingEntry = EndEncodingEntryImpl;
        }

        private void InitEncode(int keyDataType, int containerDataType)
        {
            if (!m_containerInitialized)
            {
                if (!m_containerTypeSet)
                {
                    m_rsslMap.ContainerType = containerDataType;
                    m_emaDataType = containerDataType;
                    m_containerTypeSet = true;
                }
                else if (m_rsslMap.ContainerType != containerDataType)
                {
                    throw new OmmInvalidUsageException($"Attempt to set ContianerType {containerDataType} different than Summary's type {m_rsslMap.ContainerType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }

                if (m_keyTypeSet && (m_emaKeyType != keyDataType))
                {
                    throw new OmmInvalidUsageException($"Attempt to add entry key of type {keyDataType} while the preset entry key type is {m_emaKeyType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }
                else if (!m_keyTypeSet)
                {
                    m_emaKeyType = keyDataType;
                    m_rsslMap.KeyPrimitiveType = keyDataType;
                    m_keyTypeSet = true;
                }

                var ret = m_rsslMap.EncodeInit(m_encodeIterator, 0, 0);
                while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
                {
                    ret = m_rsslMap.EncodeComplete(m_encodeIterator, false);
                    ReallocateEncodeIteratorBuffer();
                    ret = m_rsslMap.EncodeInit(m_encodeIterator, 0, 0);
                }
                if (ret < CodecReturnCode.SUCCESS)
                {
                    throw new OmmInvalidUsageException($"Failed to initialize Map encoding, return code: {ret.GetAsString()}",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_containerInitialized = true;
            }
            else
            {
                throw new OmmInvalidUsageException($"Attempt to initialize Map container when it is already initialized.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }          
        }

        private void ValidateEntryKeyAndPayload(int keyDataType, int loadDataType)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (!m_containerInitialized)
            {
                AcquireEncodeIterator();
                InitEncode(keyDataType, loadDataType);
            }
            else if (m_emaDataType != loadDataType)
            {
                throw new OmmInvalidUsageException($"Attempt to set ContainerType {loadDataType} different than Summary's type {m_rsslMap.ContainerType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
            else if (m_emaKeyType != keyDataType)
            {
                throw new OmmInvalidUsageException($"Attempt to add entry key of type {keyDataType} while the preset entry key type is {m_emaKeyType}.",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
        }

        private void AddEntryWithNoPayload(object key, int action, EmaBuffer? permissionData)
        {
            m_rsslMapEntry.Clear();
            m_rsslMapEntry.Action = (MapEntryActions)action;
            if (permissionData != null && permissionData.Length > 0)
            {
                m_rsslMapEntry.ApplyHasPermData();
                m_rsslMapEntry.PermData.Data(new ByteBuffer(permissionData.AsByteArray()).Flip());
            }
            var ret = EncodeEntry(m_emaKeyType, key);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = EncodeEntry(m_emaKeyType, key);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode MapEntry: {ret.GetAsString()}");
            }
        }

        private void AddEncodedEntry(object key, int action, EmaBuffer? permissionData, Buffer data)
        {
            m_rsslMapEntry.Clear();
            m_rsslMapEntry.Action = (MapEntryActions)action;
            if (permissionData != null && permissionData.Length > 0)
            {
                m_rsslMapEntry.ApplyHasPermData();
                m_rsslMapEntry.PermData.Data(new ByteBuffer(permissionData.AsByteArray()).Flip());
            }
            m_rsslMapEntry.EncodedData = data;
            var ret = EncodeEntry(m_emaKeyType, key);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = EncodeEntry(m_emaKeyType, key);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode MapEntry: {ret.GetAsString()}");
            }
        }

        private void StartEncodingEntry(object key, int action, EmaBuffer? permissionData)
        {
            m_rsslMapEntry.Clear();
            m_rsslMapEntry.Action = (MapEntryActions)action;
            if (permissionData != null && permissionData.Length > 0)
            {
                m_rsslMapEntry.ApplyHasPermData();
                m_rsslMapEntry.PermData.Data(new ByteBuffer(permissionData.AsByteArray()).Flip());
            }
            var ret = EncodeEntryInit(key);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = EncodeEntry(m_emaKeyType, key);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode MapEntry: {ret.GetAsString()}");
            }
        }

        private void EndEncodingEntryImpl()
        {
            var ret = m_rsslMapEntry.EncodeComplete(m_encodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to end encoding entry in Map, return code: {ret}");
            }
        }

        private CodecReturnCode EncodeEntry(int dataType, object key)
        {
            switch (dataType)
            {
                case DataType.DataTypes.INT:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Int)key);
                case DataType.DataTypes.UINT:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (UInt)key);
                case DataType.DataTypes.FLOAT:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Float)key);
                case DataType.DataTypes.DOUBLE:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Double)key);
                case DataType.DataTypes.REAL:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Real)key);
                case DataType.DataTypes.DATE:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Date)key);
                case DataType.DataTypes.TIME:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Time)key);
                case DataType.DataTypes.DATETIME:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (DateTime)key);
                case DataType.DataTypes.QOS:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Qos)key);
                case DataType.DataTypes.STATE:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (State)key);
                case DataType.DataTypes.ENUM:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Enum)key);
                case DataType.DataTypes.BUFFER:
                case DataType.DataTypes.ASCII:
                case DataType.DataTypes.UTF8:
                case DataType.DataTypes.RMTES:
                    return m_rsslMapEntry.Encode(m_encodeIterator, (Buffer)key);
                default:
                    throw new OmmInvalidUsageException($"Unsupported key primitive type: {m_emaKeyType}");
            }
        }

        private CodecReturnCode EncodeEntryInit(object key)
        {
            switch (m_emaKeyType)
            {
                case DataType.DataTypes.INT:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Int)key, 0);
                case DataType.DataTypes.UINT:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (UInt)key, 0);
                case DataType.DataTypes.FLOAT:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Float)key, 0);
                case DataType.DataTypes.DOUBLE:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Double)key, 0);
                case DataType.DataTypes.REAL:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Real)key, 0);
                case DataType.DataTypes.DATE:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Date)key, 0);
                case DataType.DataTypes.TIME:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Time)key, 0);
                case DataType.DataTypes.DATETIME:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (DateTime)key, 0);
                case DataType.DataTypes.QOS:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Qos)key, 0);
                case DataType.DataTypes.STATE:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (State)key, 0);
                case DataType.DataTypes.ENUM:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Enum)key, 0);
                case DataType.DataTypes.BUFFER:
                case DataType.DataTypes.ASCII:
                case DataType.DataTypes.UTF8:
                case DataType.DataTypes.RMTES:
                    return m_rsslMapEntry.EncodeInit(m_encodeIterator, (Buffer)key, 0);
                default:
                    throw new OmmInvalidUsageException($"Unsupported key primitive type: {m_emaKeyType}");
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
                    {
                        throw new OmmInvalidUsageException("Attempt to set SummaryData() with a ComplexType while Complete() was not called on this ComplexType.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    }
                    m_rsslMap.ApplyHasSummaryData();
                    CaptureSummaryByteBuffer(encoder);
                    m_rsslMap.EncodedSummaryData.Data(m_summaryByteBuffer!.Flip());
                } 
                else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
                {
                    m_rsslMap.ApplyHasSummaryData();
                    m_rsslMap.EncodedSummaryData = encoder!.m_encoderOwner!.m_bodyBuffer;
                }
                else
                {
                    var dataType = (summary.m_dataType >= DataType.DataTypes.REQ_MSG && summary.m_dataType <= DataType.DataTypes.GENERIC_MSG)
                                    ? DataType.DataTypes.MSG
                                    : summary.m_dataType;
                    if (dataType == DataTypes.MSG)
                    {
                        ((Msg)summary!).EncodeComplete();
                        m_rsslMap.ApplyHasSummaryData();
                        CaptureSummaryByteBuffer(encoder);
                        m_rsslMap.EncodedSummaryData.Data(m_summaryByteBuffer!.Flip());
                    }
                    else
                        throw new OmmInvalidUsageException("Attempt to set an empty SummaryData while it is not supported.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }

                m_emaDataType = summary.m_dataType;
                m_rsslMap.ContainerType = ConvertDataTypeToEta(summary.m_dataType);
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

        public void KeyType(int keyPrimitiveType)
        {
            if (m_containerInitialized)
            {
                throw new OmmInvalidUsageException("Attempt to call KeyType() when Map is already initialized.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (keyPrimitiveType >= DataType.DataTypes.RMTES || keyPrimitiveType == DataType.DataTypes.ARRAY)
            {
                throw new OmmInvalidUsageException($"The specified key type {keyPrimitiveType} is not a primitive type.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            m_emaKeyType = keyPrimitiveType;
            m_rsslMap.KeyPrimitiveType = keyPrimitiveType;
            m_keyTypeSet = true;
        }      

        public void KeyFieldId(int keyFieldId)
        {
            if (!m_containerInitialized)
            {
                m_rsslMap.ApplyHasKeyFieldId();
                m_rsslMap.KeyFieldId = keyFieldId;
            }
            else
            {
                throw new OmmInvalidUsageException("Invalid attempt to call KeyFieldId() when container is already initialized.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        public void TotalCountHint(int totalCountHint)
        {
            if (!m_containerInitialized)
            {
                m_rsslMap.ApplyHasTotalCountHint();
                m_rsslMap.TotalCountHint = totalCountHint;
            }
            else
            {
                throw new OmmInvalidUsageException("Invalid attempt to call TotalCountHint() when container is already initialized.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        public void Add(int keyDataType, object key, int action, ComplexType load, EmaBuffer? permissionData)
        {
            Encoder? encoder = load.Encoder;

            int rsslLoadDataType = ConvertDataTypeToEta(load.m_dataType);
            ValidateEntryKeyAndPayload(keyDataType, rsslLoadDataType);

            if (action == MapAction.DELETE)
            {
                AddEntryWithNoPayload(key, action, permissionData);
                return;
            }

            if (rsslLoadDataType != DataTypes.MSG)
            {
                if (encoder != null && encoder.m_encodeIterator != null && encoder.OwnsIterator())
                {
                    if (!encoder.IsComplete)
                        throw new OmmInvalidUsageException("Attempt to add a ComplexType while Complete() was not called on this ComplexType.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    AddEncodedEntry(key, action, permissionData, load.Encoder!.m_encodeIterator!.Buffer());
                }
                else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
                {
                    AddEncodedEntry(key, action, permissionData, encoder!.m_encoderOwner!.m_bodyBuffer!);
                }
                else
                { 
                    PassEncIterator(encoder);
                    StartEncodingEntry(key, action, permissionData);
                }
            }
            else
            {
                AddMessage(key, action, (Msg)load, permissionData);
            }          
        }

        private void AddMessage(object key, int action, Msg msg, EmaBuffer? permissionData)
        {
            MsgEncoder encoder = msg.m_msgEncoder!;
            if (encoder.m_encoded)
            {
                if (encoder.m_encodeIterator != null)
                {
                    encoder.EncodeComplete();
                    AddEncodedEntry(key, action, permissionData, encoder.GetEncodedBuffer(false));
                }
                else
                {
                    StartEncodingEntry(key, action, permissionData);
                    if (!encoder.EncodeComplete(m_encodeIterator!, out var error))
                        throw new OmmInvalidUsageException($"{error!}. Adding message that is not pre-encoded to Map is not supported.");
                    EndEncodingEntryImpl();
                }
            }
            else if (msg.m_hasDecodedDataSet)
            {
                AddEncodedEntry(key, action, permissionData, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException("Adding empty message as Map payload not supported.");
            }
        }

        public void AddEntryWithNoPayload(int keyDataType, object key, int action, EmaBuffer? permissionData)
        {
            ValidateEntryKeyAndPayload(keyDataType, DataType.DataTypes.NO_DATA);
            AddEntryWithNoPayload(key, action, permissionData);
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
                InitEncode(m_emaKeyType, m_emaDataType);
            }

            var ret = m_rsslMap.EncodeComplete(m_encodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to complete Map encoding, return code: {ret.GetAsString()}");
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
            if (m_returnSummaryByteBuffer) m_etaPool.ReturnByteBuffer(m_summaryByteBuffer);
            m_returnSummaryByteBuffer = false;
            m_containerInitialized = false;
            m_keyTypeSet = false;
            m_containerTypeSet = false;
            m_rsslMap.Clear();
            m_rsslMapEntry.Clear();
        }

        ~MapEncoder()
        {
            if (m_releaseIteratorByteBuffer) m_etaPool.ReturnByteBuffer(m_summaryByteBuffer);
        }
    }
}
