/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Collections;
using System.Collections.Generic;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Vector is a homogeneous container of complex data type entries. 
    /// Vector is a collection which provides iterator over the elements in this collection.
    /// 
    /// Vector entries are identified by index.
    /// 
    /// Objects of this class are intended to be short lived or rather transitional.
    /// This class is designed to efficiently perform setting and extracting of Vector and its content.
    /// Objects of this class are not cache-able.
    /// </summary>
    public sealed class Vector : ComplexType, IEnumerable<VectorEntry>
    {
        internal Eta.Codec.Vector m_rsslVector = new Eta.Codec.Vector();
        private ComplexTypeData m_summaryData;
        private DataDictionary? m_dataDictionary;

        private OmmError.ErrorCodes m_errorCode = OmmError.ErrorCodes.NO_ERROR;

        private LocalFieldSetDefDb m_localFieldSetDefDb = new LocalFieldSetDefDb();
        private LocalElementSetDefDb m_localElementSetDefDb = new LocalElementSetDefDb();
        private object? m_localDb;

        private VectorEncoder m_vectorEncoder;

        internal void SetObjectManager(EmaObjectManager objectManager)
        {
            m_objectManager = objectManager;
            m_summaryData.m_objectManager = objectManager;
        }
        
        /// <summary>
        /// Constructor for Vector
        /// </summary>
        public Vector() 
        {
            m_vectorEncoder = new VectorEncoder(this);
            Encoder = m_vectorEncoder;
            m_summaryData = new ComplexTypeData();
        }

        /// <summary>
        /// Indicates presence of TotalCountHint.
        /// </summary>
        public bool HasTotalCountHint { get => m_rsslVector.CheckHasTotalCountHint(); }

        /// <summary>
        /// Indicates the presence of summary data.
        /// </summary>
        public bool HasSummaryData { get => m_rsslVector.CheckHasSummaryData(); }

        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.VECTOR;

        /// <summary>
        /// Determines whether the current Vector instance is sortable
        /// </summary>
        /// <returns>true if sortable flag is set; false otherwise</returns>
        public bool Sortable() { return m_rsslVector.CheckSupportsSorting(); }

        /// <summary>
        /// Returns TotalCountHint
        /// </summary>
        /// <returns>the total count hint</returns>
        public int TotalCountHint() 
        { 
            if (!m_rsslVector.CheckHasTotalCountHint())
            {
                throw new OmmInvalidUsageException("Vector instance has no TotalCountHint set.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_USAGE);
            }
            return m_rsslVector.TotalCountHint;
        }

        /// <summary>
        /// Returns the contained summaryData Data based on the summaryData DataType.
        /// SummaryData contains no data if <see cref="ComplexTypeData.DataType"/> returns 
        /// <see cref="DataType.DataTypes.NO_DATA"/>
        /// </summary>
        /// <returns><see cref="Access.ComplexTypeData"/> instance</returns>
        public ComplexTypeData SummaryData() { return m_summaryData; }

        /// <summary>
        /// Clears the Vector. Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        public Vector Clear() 
        {
            ClearInt();

            return this;
        }

        internal override void ClearInt()
        {
            base.ClearInt();
            m_rsslVector.Clear();
            m_summaryData.Clear();
            m_dataDictionary = null;
            m_localDb = null;
        }

        /// <summary>
        /// Specifies Sortable.
        /// </summary>
        /// <param name="sortable">specifies whether this object is sortable</param>
        /// <returns>reference to the current <see cref="Vector"/> object</returns>
        public Vector Sortable(bool sortable) 
        {
            m_vectorEncoder.Sortable(sortable);
            return this;
        }

        /// <summary>
        /// Specifies TotalCountHint.
        /// </summary>
        /// <param name="totalCountHint">specifies whether this object is sortable</param>
        /// <returns>reference to the current <see cref="Vector"/> object</returns>
        public Vector TotalCountHint(int totalCountHint) 
        {
            m_vectorEncoder.TotalCountHint(totalCountHint);
            return this;
        }

        /// <summary>
        /// Specifies the SummaryData OMM Data.
        /// </summary>
        /// <param name="data">specifies complex type as summaryData</param>
        /// <returns>reference to current <see cref="Vector"/> instance</returns>
        public Vector SummaryData(ComplexType data) 
        {
            m_summaryData.Clear();
            m_summaryData.m_data = data;
            m_vectorEncoder.SummaryData(data);
            return this;
        }

        /// <summary>
		/// Implementation of <see cref="IEnumerable{VectorEntry}"/> interface method
		/// </summary>
		/// <returns><see cref="IEnumerator{VectorEntry}"/> instance that iterates through current Vector's entries.</returns>
        public IEnumerator<VectorEntry> GetEnumerator()
        {
            if (m_errorCode != OmmError.ErrorCodes.NO_ERROR || !m_hasDecodedDataSet)
            {
                var errorEnumerator = m_objectManager != null ? m_objectManager!.GetVectorErrorEnumerator() : new VectorErrorEnumerator();
                errorEnumerator.SetError(m_errorCode);
                return errorEnumerator;
            }
            var enumerator = m_objectManager != null ? m_objectManager!.GetVectorEnumerator() : new VectorEnumerator();
            if (!enumerator.SetData(m_MajorVersion, m_MinorVersion, 
                m_bodyBuffer!, 
                m_dataDictionary, 
                m_rsslVector.ContainerType,
                m_localDb))
            {
                enumerator.Dispose();
                var errorEnumerator = m_objectManager!.GetVectorErrorEnumerator();
                errorEnumerator.SetError(enumerator.m_decodingError.ErrorCode);
                return errorEnumerator;
            }
            return enumerator;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        internal override CodecReturnCode Decode(Eta.Codec.DecodeIterator dIter)
        {
            throw new NotImplementedException();
        }

        internal override CodecReturnCode Decode(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
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
                return ret;
            }
            ret = m_rsslVector.Decode(m_decodeIterator);
            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_errorCode = OmmError.ErrorCodes.NO_ERROR;
                    m_rsslVector.Flags = 0;
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

            if (m_rsslVector.CheckHasSetDefs())
            {
                switch (m_rsslVector.ContainerType)
                {
                    case DataTypes.FIELD_LIST:
                        m_localFieldSetDefDb.Clear();
                        m_localFieldSetDefDb.Decode(m_decodeIterator);
                        m_localDb = m_localFieldSetDefDb;
                        break;
                    case DataTypes.ELEMENT_LIST:
                        m_localElementSetDefDb.Clear();
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

            if (m_rsslVector.CheckHasSummaryData())
            {
                return m_summaryData.Decode(m_rsslVector.EncodedSummaryData, 
                    m_rsslVector.ContainerType, 
                    m_MajorVersion, m_MinorVersion, 
                    m_dataDictionary,
                    m_localDb);
            }
            return ret;
        }

        /// <summary>
		/// Completes encoding of the Vector entries
		/// </summary>
		/// <returns>reference to current <see cref="Vector"/> object</returns>
		public Vector Complete() 
        {
            m_vectorEncoder.Complete();
            return this; 
        }

        /// <summary>
        /// Adds entry to current Vector object.
        /// In case a message type is added, the container expects that 
        /// the message is either pre-encoded or contains pre-encoded payload and attributes.
        /// </summary>
        /// <param name="position">position of the entry</param>
        /// <param name="action">entry action</param>
        /// <param name="value">entry value</param>
        /// <param name="permissionData">permission data</param>
        /// <returns>reference to current <see cref="Vector"/> object</returns>
        public Vector Add(uint position, int action, ComplexType value, EmaBuffer? permissionData) 
        {
            m_vectorEncoder.Add(position, action, value, permissionData);
            return this;
        }

        /// <summary>
        /// Adds entry to current Vector object
        /// </summary>
        /// <param name="position">position of the entry</param>
        /// <param name="action">entry action</param>
        /// <param name="permissionData">permission data</param>
        /// <returns>reference to current <see cref="Vector"/> object</returns>
        public Vector Add(uint position, int action, EmaBuffer? permissionData )
        {
            m_vectorEncoder.Add(position, action, permissionData);
            return this;
        }

        internal override string ToString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent).Append("Vector");

            m_ToString.Append(" sortable=\"").Append(Sortable() ? "true" : "false").Append("\"");

            if (m_rsslVector.CheckHasTotalCountHint())
                m_ToString.Append(" totalCountHint=\"").Append(m_rsslVector.TotalCountHint).Append("\"");

            if (m_rsslVector.CheckHasSummaryData())
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

            foreach (VectorEntry vectorEntry in this)
            {
                var load = vectorEntry.Load;
                if (load == null)
                    return "\nDecoding of just encoded object in the same application is not supported\n";

                Utilities.AddIndent(m_ToString.Append("\n"), indent).Append("VectorEntry action=\"")
                        .Append(vectorEntry.VectorActionAsString()).Append("\" index=\"").Append(vectorEntry.Position);

                if (vectorEntry.HasPermissionData)
                {
                    m_ToString.Append(" permissionData=\"");
                    Utilities.AsHexString(m_ToString, vectorEntry.PermissionData).Append("\"");
                }

                m_ToString.Append(" dataType=\"").Append(Access.DataType.AsString(load.DataType)).Append("\"\n");

                ++indent;
                m_ToString.Append(load.ToString(indent));
                --indent;

                Utilities.AddIndent(m_ToString, indent).Append("VectorEntryEnd");
            }

            --indent;

            Utilities.AddIndent(m_ToString.Append("\n"), indent).Append("VectorEnd\n");

            return m_ToString.ToString();
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="Vector"/> instance.</returns>
        public override string ToString()
        {
            return ToString(0);
        }
    }

    internal class VectorErrorEnumerator : Decoder, IEnumerator<VectorEntry>
    {
        private VectorEntry m_vectorEntry = new VectorEntry();

        public VectorEntry Current => m_decodingStarted ? m_vectorEntry : null!;

        object? IEnumerator.Current => m_decodingStarted ? m_vectorEntry : null;

        public VectorErrorEnumerator()
        {
            m_dataType = DataType.DataTypes.VECTOR;
            m_isErrorDecoder = true;
        }

        public VectorErrorEnumerator(OmmError.ErrorCodes errorCode) : this()
        {
            m_decodingError.ErrorCode = errorCode;
            m_vectorEntry!.Load = m_decodingError;
        }

        public void Dispose()
        {
            Reset();
            ReturnToPool();
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
            m_vectorEntry.Clear();
        }
    }
}
