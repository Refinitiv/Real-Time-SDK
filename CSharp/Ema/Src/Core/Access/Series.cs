/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023, 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Series is a homogeneous container of complex data type entries.<br/>
    /// Series is a collection which provides iterator over the elements in this collection.
    /// </summary>
    /// <remarks>
    /// Series entries have no explicit identification. They are implicitly indexedinside Series.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and extracting of Map and its content.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// </remarks>
    public sealed class Series : ComplexType, IEnumerable<SeriesEntry>
    {
        internal Eta.Codec.Series m_rsslSeries = new Eta.Codec.Series();
        private ComplexTypeData m_summaryData;

        private LocalFieldSetDefDb? m_localFieldSetDefDb;
        private LocalElementSetDefDb? m_localElementSetDefDb;
        private object? m_localDb;

        private SeriesEncoder m_seriesEncoder;

        internal void SetObjectManager(EmaObjectManager objectManager)
        {
            m_objectManager = objectManager;
            m_summaryData.m_objectManager = objectManager;
        }

        /// <summary>
        /// Constructor for Series class
        /// </summary>
        public Series()
        {
            m_seriesEncoder = new SeriesEncoder(this);
            Encoder = m_seriesEncoder;
            m_summaryData = new ComplexTypeData();
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this); ;
            ClearTypeSpecific_All = ClearSeries_All;
            ClearTypeSpecific_Decode = ClearSeries_Decode;
            m_dataType = Access.DataType.DataTypes.SERIES;
            DecodeComplexType = DecodeSeries;
        }

        /// <summary>
        /// Indicates presence of TotalCountHint.
        /// </summary>
        public bool HasTotalCountHint { get => m_rsslSeries.CheckHasTotalCountHint(); }

        /// <summary>
        /// Returns TotalCountHint
        /// </summary>
        /// <returns>total count hint</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int TotalCountHint()
        {
            if (!m_rsslSeries.CheckHasTotalCountHint())
            {
                throw new OmmInvalidUsageException("Series instance has no TotalCountHint set.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_USAGE);
            }
            return m_rsslSeries.TotalCountHint;
        }

        /// <summary>
        /// Returns the contained summaryData Data based on the summaryData DataType.
        /// SummaryData contains no data if <see cref="ComplexTypeData.DataType"/> returns
        /// <see cref="DataType.DataTypes.NO_DATA"/>
        /// </summary>
        /// <returns><see cref="Access.ComplexTypeData"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ComplexTypeData SummaryData()
        { return m_summaryData; }

        /// <summary>
        /// Clears the Series. Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>Reference to current <see cref="Series"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Series Clear()
        {
            Clear_All();
            return this;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ClearSeries_All()
        {
            m_rsslSeries.Clear();
            m_summaryData.Clear();
            m_dataDictionary = null;
            m_localDb = null;
            m_localFieldSetDefDb?.Clear();
            m_localElementSetDefDb?.Clear();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ClearSeries_Decode()
        {
            m_summaryData.Clear();
            m_dataDictionary = null;
            m_localDb = null;
            m_localFieldSetDefDb?.Clear();
            m_localElementSetDefDb?.Clear();
        }

        /// <summary>
        /// Specifies TotalCountHint.
        /// </summary>
        /// <param name="totalCountHint">total count hint value to be set</param>
        /// <returns>Reference to current <see cref="Series"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Series TotalCountHint(int totalCountHint)
        {
            m_seriesEncoder.TotalCountHint(totalCountHint);
            return this;
        }

        /// <summary>
        /// Specifies the SummaryData OMM Data.
        /// </summary>
        /// <param name="data">specifies complex type as summaryData</param>
        /// <returns>Reference to current <see cref="Series"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Series SummaryData(ComplexType data)
        {
            m_summaryData.Clear();
            m_summaryData.m_data = data;
            m_seriesEncoder.SummaryData(data);
            return this;
        }

        /// <summary>
		/// Implementation of <see cref="IEnumerable{SeriesEntry}"/> interface method
		/// </summary>
		/// <returns><see cref="IEnumerator{SeriesEntry}"/> instance that iterates through current Series' entries.</returns>
        /// <exception cref="OmmInvalidUsageException"> is thrown if an error occured while performing enumerator setup.</exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public IEnumerator<SeriesEntry> GetEnumerator()
        {
            if (m_errorCode != OmmError.ErrorCodes.NO_ERROR || !m_hasDecodedDataSet)
            {
                var errorEnumerator = m_objectManager!.GetSeriesErrorEnumerator();
                errorEnumerator.SetError(m_errorCode);
                return errorEnumerator;
            }

            var enumerator = m_objectManager!.GetSeriesEnumerator();
            if (!enumerator.SetData(m_MajorVersion, m_MinorVersion, m_bodyBuffer!, m_dataDictionary, m_rsslSeries.ContainerType))
            {
                enumerator.Dispose();
                var errorEnumerator = m_objectManager!.GetSeriesErrorEnumerator();
                errorEnumerator.SetError(enumerator.m_decodingError.ErrorCode);
                return errorEnumerator;
            }
            return enumerator;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode DecodeSeries(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            m_hasDecodedDataSet = true;

            m_MajorVersion = majorVersion;
            m_MinorVersion = minorVersion;
            m_bodyBuffer = body;
            m_dataDictionary = dictionary;
            m_decodeIterator.Clear();
            CodecReturnCode ret;
            if ((ret = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion)) < CodecReturnCode.SUCCESS)
            {
                m_errorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
                return ret;
            }
            ret = m_rsslSeries.Decode(m_decodeIterator);
            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    m_rsslSeries.Flags = 0;
                    break;

                case CodecReturnCode.SUCCESS:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    break;

                case CodecReturnCode.ITERATOR_OVERRUN:
                    m_errorCode = OmmError.ErrorCodes.ITERATOR_OVERRUN;
                    return ret;

                case CodecReturnCode.INCOMPLETE_DATA:
                    m_errorCode = OmmError.ErrorCodes.INCOMPLETE_DATA;
                    return ret;

                default:
                    m_errorCode = OmmError.ErrorCodes.UNKNOWN_ERROR;
                    return ret;
            }

            if (m_rsslSeries.CheckHasSetDefs())
            {
                switch (m_rsslSeries.ContainerType)
                {
                    case DataTypes.FIELD_LIST:

                        if (m_localFieldSetDefDb != null)
                        {
                            m_localFieldSetDefDb.Clear();
                        }
                        else
                        {
                            m_localFieldSetDefDb = new();
                        }

                        m_localFieldSetDefDb.Decode(m_decodeIterator);
                        m_localDb = m_localFieldSetDefDb;
                        break;

                    case DataTypes.ELEMENT_LIST:

                        if( m_localElementSetDefDb != null)
                        {
                            m_localElementSetDefDb.Clear();
                        }
                        else
                        {
                            m_localElementSetDefDb = new();
                        }

                        m_localElementSetDefDb.Decode(m_decodeIterator);
                        m_localDb = m_localElementSetDefDb;
                        break;

                    default:
                        m_localDb = null;
                        m_errorCode = OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE;
                        return CodecReturnCode.FAILURE;
                }
            }
            else
            {
                m_localDb = null;
            }

            if (m_rsslSeries.CheckHasSummaryData())
            {
                return m_summaryData.Decode(m_rsslSeries.EncodedSummaryData,
                    m_rsslSeries.ContainerType,
                    m_MajorVersion, m_MinorVersion,
                    m_dataDictionary,
                    m_localDb);
            }
            return ret;
        }

        /// <summary>
		/// Completes encoding of the Series entries
		/// </summary>
		/// <returns>Reference to current <see cref="Series"/> object.</returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Series Complete()
        {
            m_seriesEncoder.Complete();
            return this;
        }

        /// <summary>
        /// Adds entry to current Series object.<br/>
        /// In case a message type is added, the container expects that the message is either pre-encoded
        /// or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="value"><see cref="ComplexType"/> value</param>
        /// <returns>Reference to current <see cref="Series"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Series AddEntry(ComplexType value)
        {
            m_seriesEncoder.Add(value);
            return this;
        }

        /// <summary>
        /// Adds entry without data to current Series object
        /// </summary>
        /// <returns>Reference to current <see cref="Series"/> object.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Series AddNoDataEntry()
        {
            m_seriesEncoder.Add();
            return this;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override string FillString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent).Append("Series");

            if (m_rsslSeries.CheckHasTotalCountHint())
            {
                m_ToString.Append(" totalCountHint=\"").Append(m_rsslSeries.TotalCountHint).Append("\"");
            }

            if (m_rsslSeries.CheckHasSummaryData())
            {
                ++indent;
                Utilities.AddIndent(m_ToString.AppendLine(), indent)
                    .Append("SummaryData dataType=\"")
                    .Append(Access.DataType.AsString(m_summaryData.DataType))
                    .Append("\"")
                    .AppendLine();

                ++indent;
                m_ToString.Append(m_summaryData.ToString(indent));
                --indent;

                Utilities.AddIndent(m_ToString, indent).Append("SummaryDataEnd");
                --indent;
            }

            ++indent;

            foreach (SeriesEntry seriesEntry in this)
            {
                var load = seriesEntry.Load;

                if (load == null)
                    return $"{NewLine}ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.{NewLine}";

                Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("SeriesEntry dataType=\"").Append(Access.DataType.AsString(load.DataType)).Append("\"").AppendLine();

                ++indent;
                m_ToString.Append(load.ToString(indent));
                --indent;
                
                Utilities.AddIndent(m_ToString, indent).Append("SeriesEntryEnd");
            }

            --indent;
            Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("SeriesEnd").AppendLine();

            return m_ToString.ToString();
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="Series"/> object.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal SeriesEncoder GetEncoder()
        {
            return m_seriesEncoder;
        }
    }

    internal class SeriesErrorEnumerator : Decoder, IEnumerator<SeriesEntry>
    {
        private SeriesEntry m_seriesEntry = new SeriesEntry();

        public SeriesEntry Current => m_decodingStarted ? m_seriesEntry : null!;

        object? IEnumerator.Current => m_decodingStarted ? m_seriesEntry : null;

        public SeriesErrorEnumerator()
        {
            m_dataType = DataType.DataTypes.SERIES;
            m_isErrorDecoder = true;
        }

        public SeriesErrorEnumerator(OmmError.ErrorCodes errorCode) : this()
        {
            m_decodingError.ErrorCode = errorCode;
            m_seriesEntry.Load = m_decodingError;
        }

        internal void SetErrorCode(OmmError.ErrorCodes errorCode)
        {
            m_decodingError.ErrorCode = errorCode;
            m_seriesEntry!.Load = m_decodingError;
        }

        public void Dispose()
        {
            Reset();
            ReturnSeriesErrorEnumeratorToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReturnSeriesErrorEnumeratorToPool()
        {
            if (!m_inPool)
            {
                m_objectManager!.ReturnToEnumeratorPool(this);
            }
        }

        public bool MoveNext()
        {
            if (m_atEnd) return false;

            m_decodingStarted = true;
            m_atEnd = true;

            return true;
        }

        public void Reset()
        {
            m_decodingStarted = false;
            m_atEnd = false;
            m_seriesEntry.Clear();
        }
    }
}