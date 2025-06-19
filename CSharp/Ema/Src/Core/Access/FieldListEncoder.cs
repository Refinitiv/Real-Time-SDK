/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access
{
    internal sealed class FieldListEncoder : Encoder
    {
        internal Eta.Codec.FieldList m_rsslFieldList = new Eta.Codec.FieldList();
        internal Eta.Codec.FieldEntry m_rsslFieldEntry = new Eta.Codec.FieldEntry();
        internal Eta.Codec.Enum m_rsslEnumValue = new Eta.Codec.Enum();

        private bool m_containerInitialized = false;

        private Func<object, CodecReturnCode>?[] m_primitiveTypeEncoding;

        internal FieldListEncoder(FieldList encoderOwner)
        {
            m_encoderOwner = encoderOwner;
            m_rsslFieldList = encoderOwner.m_rsslFieldList;
            m_primitiveTypeEncoding = new Func<object, CodecReturnCode>?[20]
            {
                null, null, null, // 0 - 2
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Int)value), // Int = 3
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (UInt)value), // UInt = 4
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Float)value), // Float = 5
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Double)value), // Double = 6
                null,
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Real)value), // Real = 8
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Date)value), // Date = 9
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Time)value), // Time = 10
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (DateTime)value), // DateTime = 11
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Qos)value), // Qos = 12
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (State)value), // State = 13
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Enum)value), // Enum = 14
                null,                                                           // Array = 15
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Buffer)value), // Buffer = 16
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Buffer)value), // Ascii = 17
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Buffer)value), // Utf8 = 18
                value => m_rsslFieldEntry.Encode(m_encodeIterator, (Buffer)value), // RMTES = 19
            };

            EndEncodingEntry = EndEncodingEntryImpl;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Info(int dictionaryId, int fieldListNum)
        {
            if (!m_containerInitialized)
            {
                m_rsslFieldList.ApplyHasInfo();
                m_rsslFieldList.FieldListNum = fieldListNum;
                m_rsslFieldList.DictionaryId = dictionaryId;
            }
            else
            {
                throw new OmmInvalidUsageException($"Invalid attempt to call Info() when container is initialized.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void InitEncode()
        {
            CodecReturnCode ret = m_rsslFieldList.EncodeInit(m_encodeIterator, null, 0);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ret = m_rsslFieldList.EncodeComplete(m_encodeIterator, false);
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslFieldList.EncodeInit(m_encodeIterator, null, 0);
            }
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to initialize FieldList encoding, return code: {ret.GetAsString()}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            m_containerInitialized = true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Complete()
        {
            if (m_containerComplete)
            {
                return;
            }

            InitContainer();

            var ret = m_rsslFieldList.EncodeComplete(m_encodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to complete FieldList encoding, return code: {ret.GetAsString()}");
            }

            if (!OwnsIterator() && m_iteratorOwner != null)
            {
                m_iteratorOwner!.EndEncodingEntry!();
            }

            m_containerComplete = true;
        }

        #region Entry encoding implementation 

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddEncodedEntry(int fieldId, int dataType, Buffer entryBuffer)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.");
            }

            m_rsslFieldEntry.Clear();
            m_rsslFieldEntry.DataType = dataType;
            m_rsslFieldEntry.FieldId = fieldId;
            m_rsslFieldEntry.EncodedData = entryBuffer;

            var ret = m_rsslFieldEntry.Encode(m_encodeIterator);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslFieldEntry.Encode(m_encodeIterator);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode FieldEntry: {ret.GetAsString()}");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void StartEncodingEntry(int fieldId, int dataType)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.");
            }

            m_rsslFieldEntry.Clear();
            m_rsslFieldEntry.DataType = dataType;
            m_rsslFieldEntry.FieldId = fieldId;

            var ret = m_rsslFieldEntry.EncodeInit(m_encodeIterator, 0);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslFieldEntry.Encode(m_encodeIterator);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode FieldEntry: {ret.GetAsString()}");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void EndEncodingEntryImpl()
        {
            var ret = m_rsslFieldEntry.EncodeComplete(m_encodeIterator, true);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslFieldEntry.EncodeComplete(m_encodeIterator, true);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode FieldEntry: {ret.GetAsString()}");
            }
        }

        #endregion

        #region Encode Primitive type entries

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddInt(int fieldId, Int value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.INT, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddUInt(int fieldId, UInt value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.UINT, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddFloat(int fieldId, Float value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.FLOAT, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddDouble(int fieldId, Double value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.DOUBLE, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddReal(int fieldId, Real value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.REAL, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddDate(int fieldId, Date value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.DATE, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddTime(int fieldId, Time value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.TIME, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddDateTime(int fieldId, DateTime value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.DATETIME, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddQos(int fieldId, Qos value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.QOS, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddState(int fieldId, State value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.STATE, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddEnum(int fieldId, Enum value)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.ENUM, value);
        }

        // Used for Buffer, ASCII, UTF8 and RMTES
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddBuffer(int fieldId, Buffer value, int bufferType)
        {
            InitContainer();

            AddPrimitiveTypeEntry(fieldId, bufferType, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddPrimitiveTypeEntry(int fieldId, int dataType, object value)
        {
            m_rsslFieldEntry.Clear();
            m_rsslFieldEntry.DataType = dataType;
            m_rsslFieldEntry.FieldId = fieldId;

            CodecReturnCode ret;
            ret = m_primitiveTypeEncoding[dataType]!(value);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_primitiveTypeEncoding[dataType]!(value);
            }

            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode primitive value: {ret.GetAsString()}");
            }
        }

        #endregion

        #region Encode Complex type and Array type entries

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddArray(int fieldId, OmmArray array)
        {
            InitContainer();

            AddComplexTypePreencodedEntry(fieldId, array, "Attempt to call AddArray() while OmmArray.Complete() was not called.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddElementList(int fieldId, ElementList elementList)
        {
            InitContainer();

            AddComplexTypePreencodedEntry(fieldId, elementList, "Attempt to call AddElementList() while ElementList.Complete() was not called.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddFieldList(int fieldId, FieldList fieldList)
        {
            InitContainer();

            AddComplexTypePreencodedEntry(fieldId, fieldList, "Attempt to call AddFieldList() while FieldList.Complete() was not called.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddMap(int fieldId, Map map)
        {
            InitContainer();

            AddComplexTypePreencodedEntry(fieldId, map, "Attempt to call AddMap() while Map.Complete() was not called.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddVector(int fieldId, Vector vector)
        {
            InitContainer();

            AddComplexTypePreencodedEntry(fieldId, vector, "Attempt to call AddVector() while Vector.Complete() was not called.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddSeries(int fieldId, Series series)
        {
            InitContainer();

            AddComplexTypePreencodedEntry(fieldId, series, "Attempt to call AddSeries() while Series.Complete() was not called.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddFilterList(int fieldId, FilterList filterList)
        {
            InitContainer();

            AddComplexTypePreencodedEntry(fieldId, filterList, "Attempt to call AddFilterList() while FilterList.Complete() was not called.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddOpaque(int fieldId, OmmOpaque opaque)
        {
            InitContainer();

            AddComplexTypeNonEmptyEntry(fieldId, opaque, "Attempt to pass an empty OmmOpaque to AddOpaque() while it is not supported.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddXml(int fieldId, OmmXml xml)
        {
            InitContainer();

            AddComplexTypeNonEmptyEntry(fieldId, xml, "Attempt to pass an empty OmmXml to AddXml() while it is not supported.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddJson(int fieldId, OmmJson json)
        {
            InitContainer();

            AddComplexTypeNonEmptyEntry(fieldId, json, "Attempt to pass an empty OmmJson to AddJson() while it is not supported.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddAnsiPage(int fieldId, OmmAnsiPage ansiPage)
        {
            InitContainer();

            AddComplexTypeNonEmptyEntry(fieldId, ansiPage, "Attempt to pass an empty OmmAnsiPage to AddAnsiPage() while it is not supported.");
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddNoData(int fieldId)
        {
            InitContainer();

            Buffer buffer = new Buffer();
            buffer.Clear();

            AddEncodedEntry(fieldId, DataType.DataTypes.NO_DATA, buffer);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddBlankPrimitive(int fieldId, int dataType)
        {
            InitContainer();

            Buffer buffer = new Buffer();
            buffer.Clear();

            AddEncodedEntry(fieldId, dataType, buffer);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddComplexTypePreencodedEntry(int fieldId, Data data, string incompleteEncoderErrorMessage)
        {
            var encoder = data.Encoder;
            var dataType = data.DataType;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(fieldId, dataType, encoder.m_encodeIterator!.Buffer());
                }
                else
                {
                    throw new OmmInvalidUsageException(incompleteEncoderErrorMessage);
                }
            }
            else if (encoder?.m_encoderOwner?.m_hasDecodedDataSet ?? false)
            {
                AddEncodedEntry(fieldId, dataType, encoder.m_encoderOwner.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(fieldId, dataType);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddComplexTypeNonEmptyEntry(int fieldId, Data data, string emptyDataErrorMessage)
        {
            var encoder = data.Encoder;
            var dataType = data.DataType;
            if (encoder != null && encoder.OwnsIterator())
            {
                AddEncodedEntry(fieldId, dataType, encoder!.m_encodeIterator!.Buffer());
            }
            else if (encoder?.m_encoderOwner?.m_hasDecodedDataSet ?? false)
            {
                AddEncodedEntry(fieldId, dataType, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException(emptyDataErrorMessage, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        #endregion

        #region Encode Msg type entries

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddMessage(int fieldId, Msg msg)
        {
            InitContainer();

            MsgEncoder encoder = msg.m_msgEncoder!;
            if (encoder.m_encoded)
            {
                if (encoder.m_encodeIterator != null)
                {
                    encoder.EncodeComplete();
                    AddEncodedEntry(fieldId, DataType.DataTypes.MSG, encoder.GetEncodedBuffer(false));
                }
                else
                {
                    StartEncodingEntry(fieldId, DataType.DataTypes.MSG);
                    if (!encoder.EncodeComplete(m_encodeIterator!, out var error))
                        throw new OmmInvalidUsageException($"{error!}. Adding message that is not pre-encoded to container is not supported.");
                    EndEncodingEntryImpl();
                }
            }
            else if (msg.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.MSG, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException("Adding empty message as container payload not supported.");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddRequestMsg(int fieldId, RequestMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddRefreshMsg(int fieldId, RefreshMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddStatusMsg(int fieldId, StatusMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddUpdateMsg(int fieldId, UpdateMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddPostMsg(int fieldId, PostMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddAckMsg(int fieldId, AckMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddGenericMsg(int fieldId, GenericMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        #endregion

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void Clear()
        {
            base.Clear();
            m_containerInitialized = false;
            m_rsslFieldList.Clear();
            m_rsslFieldEntry.Clear();
        }

        #region Utilities

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void InitContainer()
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }
        }

        #endregion
    }
}
