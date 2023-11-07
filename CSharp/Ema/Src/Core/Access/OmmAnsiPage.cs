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
    /// <summary>
    /// OmmAnsiPage represents AnsiPage data format in Omm.
    /// </summary>
    public sealed class OmmAnsiPage : ComplexType
    {
        private OmmBuffer m_buffer = new OmmBuffer();
        private OmmNonRwfEncoder m_nonRWFEncoder;

        /// <summary>
        /// Constructor for OmmAnsiPage class
        /// </summary>
        public OmmAnsiPage()
        {
            m_nonRWFEncoder = new OmmNonRwfEncoder();
            Encoder = m_nonRWFEncoder;
            Encoder.m_encoderOwner = this;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            ClearTypeSpecific_All = ClearAnsiPage_All;
            ClearTypeSpecific_Decode = ClearAnsiPage_All;
            DecodePrimitiveType = DecodeAnsiPagePrimitive;
            DecodeComplexType = DecodeAnsiPageComplex;
            m_dataType = Access.DataType.DataTypes.ANSI_PAGE;
        }

        /// <summary>
        /// Returns contained AnsiPage buffer.
        /// </summary>
        public EmaBuffer Buffer { get => m_buffer.Value; }

        /// <summary>
        /// Returns a string representation of the class instance.
        /// </summary>
        /// <returns>string representation of the class object.</returns>
        public string? GetString() { return m_buffer!.Value!.ToString(); }

        /// <summary>
        /// Clears the OmmAnsiPage. 
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>Reference to the current <see cref="OmmAnsiPage"/> object.</returns>
        public OmmAnsiPage Clear() 
        {
            Clear_All();
            return this;
        }

        internal void ClearAnsiPage_All()
        {
            m_buffer.Clear_All();
        }

        /// <summary>
        /// Specifies Set.
        /// </summary>
        /// <param name="value">specifies AnsiPage data using SetString</param>
        /// <returns>Reference to the current <see cref="OmmAnsiPage"/> object.</returns>
        public OmmAnsiPage SetString(string value)
        {
            m_nonRWFEncoder.EncodeBuffer(value);
            return this;
        }

        /// <summary>
        /// Specifies Set.
        /// </summary>
        /// <param name="value">specifies AnsiPage data using <see cref="EmaBuffer"/></param>
        /// <returns>Reference to the current <see cref="OmmAnsiPage"/> object.</returns>
        public OmmAnsiPage SetBuffer(EmaBuffer value)
        {
            m_nonRWFEncoder.EncodeBuffer(value);
            return this;
        }

        internal CodecReturnCode DecodeAnsiPagePrimitive(DecodeIterator dIter)
        {
            if (m_buffer.DecodeOmmBuffer(dIter) < CodecReturnCode.SUCCESS)
            {
                Code = DataCode.BLANK;
            }
            else
            {
                Code = DataCode.NO_CODE;
            }
            return CodecReturnCode.SUCCESS;
        }

        internal CodecReturnCode DecodeAnsiPageComplex(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
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
            if ((ret = m_buffer.DecodeOmmBuffer(m_decodeIterator)) < CodecReturnCode.SUCCESS)
            {
                Code = DataCode.BLANK;
            }
            else
            {
                Code = DataCode.NO_CODE;
            }
            return ret;
        }

        internal override string ToString(int indent)
        {
            m_ToString.Length = 0;

            Utilities.AddIndent(m_ToString, indent);
            m_ToString.Append("AnsiPage").AppendLine().Append(EmaUtility.AsHexString(AsHex()));
            Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("AnsiPageEnd").AppendLine();

            return m_ToString.ToString();
        }
    }
}
