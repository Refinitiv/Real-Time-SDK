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
    /// FilterList is a heterogeneous container of complex data type entries.
    /// Objects of this class are intended to be short lived or rather transitional.
    /// This class is designed to efficiently perform setting and extracting of FieldList and its content.
    /// Objects of this class are not cache-able.
    /// </summary>
    public sealed class FilterList : ComplexType, IEnumerable<FilterEntry>
    {
        internal FilterListEncoder m_filterListEncoder;
        internal Eta.Codec.FilterList m_rsslFilterList = new Eta.Codec.FilterList();

        /// <summary>
        /// Constructor for FilterList
        /// </summary>
        public FilterList()
        {
            m_filterListEncoder = new FilterListEncoder(this);
            Encoder = m_filterListEncoder;
            ClearTypeSpecific_All = ClearFilterList_All;
            ClearTypeSpecific_Decode = ClearFilterList_Decode;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            DecodeComplexType = DecodeFilterList;
            m_dataType = Access.DataType.DataTypes.FILTER_LIST;
        }

        /// <summary>
        /// Indicates presence of TotalCountHint.
        /// </summary>
        public bool HasTotalCountHint { get => m_rsslFilterList.CheckHasTotalCountHint(); }

        /// <summary>
        /// Returns TotalCountHint
        /// </summary>
        /// <returns><see cref="int"/> value representing TotalCountHint</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if HasTotalCountHint returns false</exception>
        public int TotalCountHint()
        {
            if (!m_rsslFilterList.CheckHasTotalCountHint())
            {
                throw new OmmInvalidUsageException("Invalid usage: FilterList has no TotalCountHint property.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_USAGE);
            }

            return m_rsslFilterList.TotalCountHint;
        }

        /// <summary>
        /// Clears the FilterList.
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>Reference to current <see cref="FilterList"/> object.</returns>
        public FilterList Clear()
        {
            Clear_All();
            return this;
        }

        internal void ClearFilterList_All()
        {
            m_dataDictionary = null;
            m_rsslFilterList.Clear();
        }

        internal void ClearFilterList_Decode()
        {
            m_dataDictionary = null;
        }

        /// <summary>
		/// Implementation of <see cref="IEnumerable{FilterEntry}"/> interface method
		/// </summary>
		/// <returns><see cref="IEnumerator{FilterEntry}"/> instance that iterates through current FilterList's entries.</returns>
        public IEnumerator<FilterEntry> GetEnumerator()
        {
            if (m_errorCode != OmmError.ErrorCodes.NO_ERROR || !m_hasDecodedDataSet)
            {
                var errorEnumerator = m_objectManager!.GetFilterListErrorEnumerator();
                errorEnumerator.SetError(m_errorCode);
                return errorEnumerator;
            }

            var enumerator = m_objectManager.GetFilterListEnumerator();
            if (!enumerator.SetData(m_MajorVersion, m_MinorVersion,
                m_bodyBuffer!,
                m_dataDictionary,
                m_rsslFilterList.ContainerType))
            {
                enumerator.Dispose();
                var errorEnumerator = m_objectManager!.GetFilterListErrorEnumerator();
                errorEnumerator.SetError(m_errorCode);
                return errorEnumerator;
            }
            return enumerator;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        internal CodecReturnCode DecodeFilterList(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            m_hasDecodedDataSet = true;

            m_MajorVersion = majorVersion;
            m_MinorVersion = minorVersion;
            m_bodyBuffer = body;
            m_dataDictionary = dictionary;
            m_decodeIterator.Clear();
            CodecReturnCode ret;
            if ((ret = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion)) != CodecReturnCode.SUCCESS)
            {
                m_errorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
                return ret;
            }
            ret = m_rsslFilterList.Decode(m_decodeIterator);
            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    m_rsslFilterList.Flags = 0;
                    break;

                case CodecReturnCode.SUCCESS:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    break;

                case CodecReturnCode.ITERATOR_OVERRUN:
                    m_errorCode = OmmError.ErrorCodes.ITERATOR_OVERRUN;
                    break;

                case CodecReturnCode.INCOMPLETE_DATA:
                    m_errorCode = OmmError.ErrorCodes.INCOMPLETE_DATA;
                    break;

                default:
                    m_errorCode = OmmError.ErrorCodes.UNKNOWN_ERROR;
                    break;
            }
            return ret;
        }

        /// <summary>
        /// Specifies TotalCountHint.
        /// </summary>
        /// <param name="totalCountHint">specifies estimated total number of entries</param>
        /// <returns>Reference to the current <see cref="FilterList"/> object.</returns>
        public FilterList TotalCountHint(int totalCountHint)
        {
            m_filterListEncoder.TotalCountHint(totalCountHint);
            return this;
        }

        /// <summary>
        /// Adds next entry to current FilterList instance
        /// </summary>
        /// <param name="filterId">filter entry id</param>
        /// <param name="action">filter entry action</param>
        /// <param name="permissionData">buffer containing permission data</param>
        /// <returns>Reference to current <see cref="FilterList"/> object.</returns>
        public FilterList AddEntry(int filterId, int action, EmaBuffer? permissionData = null)
        {
            m_filterListEncoder.Add(filterId, action, permissionData);
            return this;
        }

        /// <summary>
        /// Adds next entry to current FilterList instance.
        /// In case a message type is added, the container expects that
        /// the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="filterId">filter entry id</param>
        /// <param name="action">filter entry action</param>
        /// <param name="value">entry value</param>
        /// <param name="permissionData">entry permission data</param>
        /// <returns>Reference to current <see cref="FilterList"/> object.</returns>
        public FilterList AddEntry(int filterId, int action, ComplexType value, EmaBuffer? permissionData = null)
        {
            m_filterListEncoder.Add(filterId, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Completes encoding of the FilterList entries
        /// </summary>
        /// <returns>Reference to current <see cref="FilterList"/> object.</returns>
        public FilterList Complete()
        {
            m_filterListEncoder.Complete();
            return this;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override string FillString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent)
                    .Append("FilterList");

            if (m_rsslFilterList.CheckHasTotalCountHint())
            {
                m_ToString.Append(" totalCountHint=\"")
                         .Append(m_rsslFilterList.TotalCountHint)
                         .Append("\"");
            }

            ++indent;

            foreach (FilterEntry filterEntry in this)
            {
                var load = filterEntry.Load;
                if (load == null)
                    return "\nToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.\n";

                Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("FilterEntry action=\"")
                                                                      .Append(filterEntry.FilterActionAsString())
                                                                      .Append("\" filterId=\"")
                                                                      .Append(filterEntry.FilterId)
                                                                      .Append("\"");

                if (filterEntry.HasPermissionData)
                {
                    m_ToString.Append(" permissionData=\"");
                    Utilities.AsHexString(m_ToString, filterEntry.PermissionData).Append("\"");
                }

                m_ToString.Append(" dataType=\"").Append(Access.DataType.AsString(filterEntry.LoadType))
                         .Append("\"").AppendLine();

                if (load != null)
                {
                    ++indent;
                    m_ToString.Append(load.ToString(indent));
                    --indent;
                }

                Utilities.AddIndent(m_ToString, indent).Append("FilterEntryEnd");
            }

            --indent;

            Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("FilterListEnd").AppendLine();
            return m_ToString.ToString();
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="FilterList"/> object.</returns>
        public override string ToString()
        {
            return ToString(0);
        }
    }

    internal class FilterListErrorEnumerator : Decoder, IEnumerator<FilterEntry>
    {
        private FilterEntry m_filterEntry = new FilterEntry();

        public FilterEntry Current => m_decodingStarted ? m_filterEntry : null!;

        object? IEnumerator.Current => m_decodingStarted ? m_filterEntry : null;

        public FilterListErrorEnumerator()
        {
            m_dataType = DataType.DataTypes.FILTER_LIST;
            m_isErrorDecoder = true;
        }

        public FilterListErrorEnumerator(OmmError.ErrorCodes errorCode) : this()
        {
            m_decodingError.ErrorCode = errorCode;
            m_filterEntry!.Load = m_decodingError;
        }

        internal void SetErrorCode(OmmError.ErrorCodes errorCode)
        {
            m_decodingError.ErrorCode = errorCode;
            m_filterEntry!.Load = m_decodingError;
        }

        public void Dispose()
        {
            Reset();
            ReturnFilterListErrorEnumeratorToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReturnFilterListErrorEnumeratorToPool()
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
            m_filterEntry.Clear();
        }
    }
}
