/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;
using DateTime = LSEG.Eta.Codec.DateTime;
using Double = LSEG.Eta.Codec.Double;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Class that handles ElementList encoding.
    /// Instances of this class are created by Access.ElementList 
    /// and are associated with the respective instances that created them 
    /// during their lifetime.
    /// </summary>
    internal sealed class ElementListEncoder : Encoder
    {
        private Eta.Codec.ElementList m_rsslElementList;       
        private Eta.Codec.ElementEntry m_rsslElementEntry = new Eta.Codec.ElementEntry();

        private bool m_containerInitialized = false;

        private Func<object, CodecReturnCode>?[] m_primitiveTypeEncoding;

        private Buffer buffer = new Buffer();

        internal ElementListEncoder(ElementList encoderOwner)
        {
            m_encoderOwner = encoderOwner;
            m_rsslElementList = encoderOwner.m_rsslElementList;
            
            EndEncodingEntry = EndEncodingEntryImpl;
            m_primitiveTypeEncoding = new Func<object, CodecReturnCode>?[20]
            {
                null, null, null, // 0 - 2
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Int)value), // Int = 3
                value => m_rsslElementEntry.Encode(m_encodeIterator, (UInt)value), // UInt = 4
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Float)value), // Float = 5
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Double)value), // Double = 6
                null,
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Real)value), // Real = 8
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Date)value), // Date = 9
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Time)value), // Time = 10
                value => m_rsslElementEntry.Encode(m_encodeIterator, (DateTime)value), // DateTime = 11
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Qos)value), // Qos = 12
                value => m_rsslElementEntry.Encode(m_encodeIterator, (State)value), // State = 13
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Enum)value), // Enum = 14
                null,                                                           // Array = 15
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Buffer)value), // Buffer = 16
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Buffer)value), // Ascii = 17
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Buffer)value), // Utf8 = 18
                value => m_rsslElementEntry.Encode(m_encodeIterator, (Buffer)value), // RMTES = 19
            };
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Info(int elementListNum)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasInfo();
                m_rsslElementList.ElementListNum = elementListNum;
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
            CodecReturnCode ret = m_rsslElementList.EncodeInit(m_encodeIterator, null, 0);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ret = m_rsslElementList.EncodeComplete(m_encodeIterator, false);
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslElementList.EncodeInit(m_encodeIterator, null, 0);
            }
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to initialize ElementList encoding, return code: {ret.GetAsString()}",
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

            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            var ret = m_rsslElementList.EncodeComplete(m_encodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to complete ElementList encoding, return code: {ret.GetAsString()}");
            }

            if (!OwnsIterator() && m_iteratorOwner != null)
            {
                m_iteratorOwner!.EndEncodingEntry!();
            }

            m_containerComplete = true;
        }

        #region Encode Primitive type entries

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddInt(string name, Int value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.INT, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddUInt(string name, UInt value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.UINT, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddFloat(string name, Float value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.FLOAT, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddDouble(string name, Double value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.DOUBLE, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddReal(string name, Real value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.REAL, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddDate(string name, Date value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.DATE, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddTime(string name, Time value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.TIME, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddDateTime(string name, DateTime value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.DATETIME, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddQos(string name, Qos value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.QOS, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddState(string name, State value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.STATE, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddEnum(string name, Enum value)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, DataType.DataTypes.ENUM, value);
        }

        // Used for Buffer, ASCII, UTF8 and RMTES
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddBuffer(string name, Buffer value, int bufferType)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            AddPrimitiveTypeEntry(name, bufferType, value);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddPrimitiveTypeEntry(string name, int dataType, object value)
        {
            m_rsslElementEntry.Clear();
            m_rsslElementEntry.DataType = dataType;
            m_rsslElementEntry.Name.Data(name);

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
        public void AddArray(string name, OmmArray array)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = array.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(name, DataType.DataTypes.ARRAY, encoder!.m_encodeIterator!.Buffer());
                } 
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddArray() while OmmArray.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.ARRAY, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.ARRAY);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddElementList(string name, ElementList elementList)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = elementList.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(name, DataType.DataTypes.ELEMENT_LIST, encoder!.m_encodeIterator!.Buffer());
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddElementList() while ElementList.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.ELEMENT_LIST, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.ELEMENT_LIST);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddFieldList(string name, FieldList fieldList)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = fieldList.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(name, DataType.DataTypes.FIELD_LIST, encoder!.m_encodeIterator!.Buffer());
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddFieldList() while FieldList.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.FIELD_LIST, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.FIELD_LIST);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddMap(string name, Map map)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = map.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(name, DataType.DataTypes.MAP, encoder!.m_encodeIterator!.Buffer());
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddFieldList() while Map.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.MAP, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.MAP);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddVector(string name, Vector vector)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = vector.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(name, DataType.DataTypes.VECTOR, encoder!.m_encodeIterator!.Buffer());
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddVector() while Vector.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.VECTOR, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.VECTOR);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddSeries(string name, Series series)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = series.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(name, DataType.DataTypes.SERIES, encoder!.m_encodeIterator!.Buffer());
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddSeries() while Series.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.SERIES, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.SERIES);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddFilterList(string name, FilterList filterList)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = filterList.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                if (encoder.IsComplete)
                {
                    AddEncodedEntry(name, DataType.DataTypes.FILTER_LIST, encoder!.m_encodeIterator!.Buffer());
                }
                else
                {
                    throw new OmmInvalidUsageException($"Attempt to call AddFilterList() while FilterList.Complete() was not called.");
                }
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.FILTER_LIST, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.FILTER_LIST);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddOpaque(string name, OmmOpaque opaque)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = opaque.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                AddEncodedEntry(name, DataType.DataTypes.OPAQUE, encoder!.m_encodeIterator!.Buffer());
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.OPAQUE, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.OPAQUE);

                /* throw new OmmInvalidUsageException("Attempt to pass an empty OmmOpaque to AddOpaque() while it is not supported.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION); */
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddXml(string name, OmmXml xml)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = xml.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                AddEncodedEntry(name, DataType.DataTypes.XML, encoder!.m_encodeIterator!.Buffer());
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.XML, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.XML);

                /* throw new OmmInvalidUsageException("Attempt to pass an empty OmmXml to AddXml() while it is not supported.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION); */
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddAnsiPage(string name, OmmAnsiPage ansiPage)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Encoder? encoder = ansiPage.Encoder;
            if (encoder != null && encoder.OwnsIterator())
            {
                AddEncodedEntry(name, DataType.DataTypes.ANSI_PAGE, encoder!.m_encodeIterator!.Buffer());
            }
            else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.ANSI_PAGE, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                PassEncIterator(encoder);
                StartEncodingEntry(name, DataType.DataTypes.ANSI_PAGE);

                /* throw new OmmInvalidUsageException("Attempt to pass an empty OmmAnsiPage to AddAnsiPage() while it is not supported.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION); */
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddNoData(string name)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            Buffer buffer = new Buffer();
            buffer.Clear();

            AddEncodedEntry(name, DataType.DataTypes.NO_DATA, buffer);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddBlankPrimitive(string name, int dataType)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            
            buffer.Clear();

            AddEncodedEntry(name, dataType, buffer);
        }

        #endregion

        #region Encode Msg type entries

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddMessage(string name, Msg msg)
        {
            if (!m_containerInitialized)
            {
                m_rsslElementList.ApplyHasStandardData();
                AcquireEncodeIterator();
                InitEncode();
            }

            MsgEncoder encoder = msg.m_msgEncoder!;
            if (encoder.m_encoded)
            {
                if (encoder.m_encodeIterator != null)
                {
                    encoder.EncodeComplete();
                    AddEncodedEntry(name, DataType.DataTypes.MSG, encoder.GetEncodedBuffer(false));
                }
                else
                {
                    StartEncodingEntry(name, DataType.DataTypes.MSG);
                    if (!encoder.EncodeComplete(m_encodeIterator!, out var error))
                        throw new OmmInvalidUsageException($"{error!}. Adding message that is not pre-encoded to container is not supported.");
                    EndEncodingEntryImpl();
                }
            }
            else if (msg.m_hasDecodedDataSet)
            {
                AddEncodedEntry(name, DataType.DataTypes.MSG, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException("Adding empty message as container payload not supported.");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddRequestMsg(string name, RequestMsg msg)
        {
            AddMessage(name, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddRefreshMsg(string name, RefreshMsg msg)
        {
            AddMessage(name, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddStatusMsg(string name, StatusMsg msg)
        {
            AddMessage(name, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddUpdateMsg(string name, UpdateMsg msg)
        {
            AddMessage(name, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddPostMsg(string name, PostMsg msg)
        {
            AddMessage(name, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddAckMsg(string name, AckMsg msg)
        {
            AddMessage(name, msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddGenericMsg(string name, GenericMsg msg)
        {
            AddMessage(name, msg);
        }

        #endregion

        #region Entry encoding implementation 

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddEncodedEntry(string name, int dataType, Buffer entryBuffer)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.");
            }

            m_rsslElementEntry.Clear();
            m_rsslElementEntry.DataType = dataType;
            m_rsslElementEntry.Name.Data(name);
            m_rsslElementEntry.EncodedData = entryBuffer;

            var ret = m_rsslElementEntry.Encode(m_encodeIterator);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslElementEntry.Encode(m_encodeIterator);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode ElementEntry: {ret.GetAsString()}");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void StartEncodingEntry(string name, int dataType)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.");
            }

            m_rsslElementEntry.Clear();
            m_rsslElementEntry.DataType = dataType;
            m_rsslElementEntry.Name.Data(name);

            var ret = m_rsslElementEntry.EncodeInit(m_encodeIterator, 0);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslElementEntry.Encode(m_encodeIterator);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode ElementEntry: {ret.GetAsString()}");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void EndEncodingEntryImpl()
        {
            var ret = m_rsslElementEntry.EncodeComplete(m_encodeIterator, true);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslElementEntry.EncodeComplete(m_encodeIterator, true);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode ElementEntry: {ret.GetAsString()}");
            }
        }

        #endregion

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void Clear()
        {
            base.Clear();
            m_containerInitialized = false;
            m_rsslElementList.Clear();
            m_rsslElementEntry.Clear();
        }
    }
}
