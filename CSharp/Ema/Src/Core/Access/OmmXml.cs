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
    ///  OmmXml represents XML data format in Omm.
    /// <p>Method <see cref="GetString"/> can have performance decrease, since it will trigger garbage collection.</p>
    /// <p>Objects of this class are intended to be short lived or rather transitional.</p>
    /// <p>This class is designed to efficiently perform setting and extracting of XML and its content.</p>
    /// <p>Objects of this class are not cache-able.</p>
    /// </summary>
    public sealed class OmmXml : ComplexType
    {
        private OmmBuffer m_buffer = new OmmBuffer();
        private OmmNonRwfEncoder m_nonRWFEncoder;

        /// <summary>
        /// Constructor for OmmXml class
        /// </summary>
        public OmmXml() 
        {
            m_nonRWFEncoder = new OmmNonRwfEncoder();
            Encoder = m_nonRWFEncoder;
            Encoder.m_encoderOwner = this;
        }

        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.XML;

        /// <summary>
        /// Returns contained XML buffer.
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing the XML data</returns>
        public EmaBuffer Value { get => m_buffer.Value; }
        
        /// <summary>
        /// Returns a string representation of the class instance.
        /// </summary>
        /// <returns>string representation of the class instance</returns>
        public string? GetString() { return m_buffer!.Value!.ToString(); }
        
        /// <summary>
        /// Clears the OmmXml instance.
        /// </summary>
        /// <returns>reference to current <see cref="OmmXml"/> object</returns>
        public OmmXml Clear() 
        {
            ClearInt();
            return this;
        }

        internal override void ClearInt()
        {
            base.ClearInt();
            m_buffer.ClearInt();
        }

        /// <summary>
        /// Specifies Set.
        /// </summary>
        /// <param name="value">specifies XML data using string</param>
        /// <returns>reference to current <see cref="OmmXml"/> object</returns>
        public OmmXml SetString(string value) 
        {
            m_nonRWFEncoder.EncodeBuffer(value);
            return this;
        }
        
        /// <summary>
        /// Encodes specified EmaBuffer containing XML data.
        /// </summary>
        /// <param name="value"> specifies XML data using <see cref="EmaBuffer"/></param>
        /// <returns>reference to current <see cref="OmmXml"/> object</returns>
        public OmmXml SetBuffer(EmaBuffer value) 
        {
            m_nonRWFEncoder.EncodeBuffer(value);
            return this;
        }

        internal override CodecReturnCode Decode(DecodeIterator dIter)
        {
            if (m_buffer.Decode(dIter) < CodecReturnCode.SUCCESS)
            {
                Code = DataCode.BLANK;
            } else
            {
                Code = DataCode.NO_CODE;
            }
            return CodecReturnCode.SUCCESS;
        }

        internal override CodecReturnCode Decode(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
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
            if ((ret = m_buffer.Decode(m_decodeIterator)) < CodecReturnCode.SUCCESS)
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
            Utilities.AddIndent(m_ToString, indent).Append("Xml");

            ++indent;
            Utilities.AddIndent(m_ToString.Append("\n"), indent);
            if (DataCode.BLANK == Code)
                m_ToString.Append(BLANK_STRING);
            else if (m_bodyBuffer!.Length > 0)
                m_ToString.Append(m_bodyBuffer.ToString());

            --indent;

            Utilities.AddIndent(m_ToString.Append("\n"), indent).Append("XmlEnd\n");

            return m_ToString.ToString();
        }
    }
}
