/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
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

        public void Complete()
        {
            if (m_containerComplete)
            {
                return;
            }

            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

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

        public void AddInt(int fieldId, Int value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.INT, value);
        }

        public void AddUInt(int fieldId, UInt value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.UINT, value);
        }

        public void AddFloat(int fieldId, Float value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.FLOAT, value);
        }

        public void AddDouble(int fieldId, Double value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.DOUBLE, value);
        }

        public void AddReal(int fieldId, Real value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.REAL, value);
        }

        public void AddDate(int fieldId, Date value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.DATE, value);
        }

        public void AddTime(int fieldId, Time value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.TIME, value);
        }

        public void AddDateTime(int fieldId, DateTime value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.DATETIME, value);
        }

        public void AddQos(int fieldId, Qos value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.QOS, value);
        }

        public void AddState(int fieldId, State value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.STATE, value);
        }

        public void AddEnum(int fieldId, Enum value)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, DataType.DataTypes.ENUM, value);
        }

        // Used for Buffer, ASCII, UTF8 and RMTES
        public void AddBuffer(int fieldId, Buffer value, int bufferType)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(fieldId, bufferType, value);
        }

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

        public void AddArray(int fieldId, OmmArray array)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = array.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(fieldId, DataType.DataTypes.ARRAY, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddArray() while OmmArray.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.ARRAY, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(fieldId, DataType.DataTypes.ARRAY);
            }
        }

        public void AddElementList(int fieldId, ElementList elementList)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = elementList.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(fieldId, DataType.DataTypes.ELEMENT_LIST, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddElementList() while ElementList.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.ELEMENT_LIST, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(fieldId, DataType.DataTypes.ELEMENT_LIST);
            }
        }

        public void AddFieldList(int fieldId, FieldList fieldList)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = fieldList.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(fieldId, DataType.DataTypes.FIELD_LIST, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddFieldList() while FieldList.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.FIELD_LIST, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(fieldId, DataType.DataTypes.FIELD_LIST);
            }
        }

        public void AddMap(int fieldId, Map map)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = map.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(fieldId, DataType.DataTypes.MAP, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddFieldList() while Map.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.MAP, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(fieldId, DataType.DataTypes.MAP);
            }
        }

        public void AddVector(int fieldId, Vector vector)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = vector.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(fieldId, DataType.DataTypes.VECTOR, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddVector() while Vector.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.VECTOR, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(fieldId, DataType.DataTypes.VECTOR);
            }
        }

        public void AddSeries(int fieldId, Series series)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = series.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(fieldId, DataType.DataTypes.SERIES, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddSeries() while Series.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.SERIES, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(fieldId, DataType.DataTypes.SERIES);
            }
        }

        public void AddFilterList(int fieldId, FilterList filterList)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = filterList.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(fieldId, DataType.DataTypes.FILTER_LIST, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddFilterList() while FilterList.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.FILTER_LIST, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(fieldId, DataType.DataTypes.FILTER_LIST);
            }
        }

        public void AddOpaque(int fieldId, OmmOpaque opaque)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = opaque.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.OPAQUE, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.OPAQUE, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException("Attempt to pass an empty OmmOpaque to AddOpaque() while it is not supported.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        public void AddXml(int fieldId, OmmXml xml)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = xml.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.XML, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.XML, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException("Attempt to pass an empty OmmXml to AddXml() while it is not supported.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        public void AddAnsiPage(int fieldId, OmmAnsiPage ansiPage)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = ansiPage.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.ANSI_PAGE, encoder!.m_encodeIterator!.Buffer()); // TODO: check that this buffer is ok
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(fieldId, DataType.DataTypes.ANSI_PAGE, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException("Attempt to pass an empty OmmAnsiPage to AddAnsiPage() while it is not supported.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        public void AddNoData(int fieldId)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Buffer buffer = new Buffer();
            buffer.Clear();

            AddEncodedEntry(fieldId, DataType.DataTypes.NO_DATA, buffer);
        }

        public void AddBlankPrimitive(int fieldId, int dataType)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Buffer buffer = new Buffer();
            buffer.Clear();

            AddEncodedEntry(fieldId, dataType, buffer);
        }

        #endregion

        #region Encode Msg type entries

        private void AddMessage(int fieldId, Msg msg)
        {
            if (!m_rsslFieldList.CheckHasStandardData())
            {
                m_rsslFieldList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            MsgEncoder encoder = (MsgEncoder)msg.Encoder!;
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

        public void AddRequestMsg(int fieldId, RequestMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        public void AddRefreshMsg(int fieldId, RefreshMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        public void AddStatusMsg(int fieldId, StatusMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        public void AddUpdateMsg(int fieldId, UpdateMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        public void AddPostMsg(int fieldId, PostMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        public void AddAckMsg(int fieldId, AckMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        public void AddGenericMsg(int fieldId, GenericMsg msg)
        {
            AddMessage(fieldId, msg);
        }

        #endregion

        public override void Clear()
        {
            base.Clear();
            m_containerInitialized = false;
            m_rsslFieldList.Clear();
            m_rsslFieldEntry.Clear();
        }
    }
}
