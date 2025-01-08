/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Represents any structured text format in Omm (like XML, JSON, etc).
    /// </summary>
    /// <remarks>
    /// Method <see cref="StructuredTextData.GetString"/> can have performance decrease, since it will trigger garbage collection.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and extracting of structured text and its content.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// </remarks>
    public abstract class StructuredTextData : ComplexType
    {
        protected OmmBuffer m_buffer = new();
        private OmmNonRwfEncoder m_nonRWFEncoder = new();

        /// <summary>
        /// Constructor for StructuredTextData class.
        /// </summary>
        protected StructuredTextData()
        {
            Encoder = m_nonRWFEncoder;
            Encoder.m_encoderOwner = this;
            ClearTypeSpecific_All = m_buffer.Clear_All;
            DecodePrimitiveType = DecodeOmmPrimitive;
            DecodeComplexType = DecodeOmmComplex;
        }

        /// <summary>
        /// Returns contained text buffer.
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing the text data</returns>
        public EmaBuffer Value => m_buffer.Value;

        /// <summary>
        /// Returns a string representation of the class instance.
        /// </summary>
        /// <returns>string representation of the class object.</returns>
        public string? GetString() => m_buffer.Value?.ToString();


        protected void SetString(string value) => m_nonRWFEncoder.EncodeBuffer(value);
        protected void SetBuffer(EmaBuffer value) => m_nonRWFEncoder.EncodeBuffer(value);

        internal CodecReturnCode DecodeOmmPrimitive(DecodeIterator dIter)
        {
            Code = m_buffer.DecodeOmmBuffer(dIter) < CodecReturnCode.SUCCESS
                ? DataCode.BLANK
                : DataCode.NO_CODE;
            return CodecReturnCode.SUCCESS;
        }

        internal CodecReturnCode DecodeOmmComplex(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            m_bodyBuffer = body;
            m_MajorVersion = majorVersion;
            m_MinorVersion = minorVersion;
            m_decodeIterator.Clear();
            CodecReturnCode ret;
            if ((ret = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_MajorVersion, m_MinorVersion)) != CodecReturnCode.SUCCESS)
            {
                return ret;
            }
            ret = m_buffer.DecodeOmmBuffer(m_decodeIterator);
            Code = ret < CodecReturnCode.SUCCESS
                ? DataCode.BLANK
                : DataCode.NO_CODE;
            return ret;
        }

        internal override string ToString(int indent)
        {
            m_ToString.Clear();

            var dataTypeName = LSEG.Ema.Access.DataType.AsString(DataType);

            m_ToString.AddIndent(indent).AppendLine(dataTypeName);
            ++indent;

            m_ToString.AddIndent(indent);
            if (DataCode.BLANK == Code)
                m_ToString.Append(BLANK_STRING);
            else if (m_bodyBuffer!.Length > 0)
                m_ToString.Append(m_bodyBuffer.ToString());

            --indent;
            m_ToString.AddIndent(indent, true).Append(dataTypeName).AppendLine("End");

            return m_ToString.ToString();
        }

        internal override string FillString(int indent)
        {
            throw new NotImplementedException();
        }
    }
}
