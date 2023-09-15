/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
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
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.ANSI_PAGE;

        /// <summary>
        /// Constructor for OmmAnsiPage class
        /// </summary>
        public OmmAnsiPage()
        {
            m_nonRWFEncoder = new OmmNonRwfEncoder();
            Encoder = m_nonRWFEncoder;
            Encoder.m_encoderOwner = this;
        }

        /// <summary>
        /// Returns contained AnsiPage buffer.
        /// </summary>
        public EmaBuffer Buffer { get => m_buffer.Value; }

        /// <summary>
        /// Returns a string representation of the class instance.
        /// </summary>
        /// <returns>string representation of the class instance</returns>
        public string? GetString() { return m_buffer!.Value!.ToString(); }

        /// <summary>
        /// Clears the OmmAnsiPage. 
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>reference to the current <see cref="OmmAnsiPage"/> object</returns>
        public OmmAnsiPage Clear() 
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
        /// <param name="value">specifies AnsiPage data using SetString</param>
        /// <returns>reference to the current <see cref="OmmAnsiPage"/> object</returns>
        public OmmAnsiPage SetString(string value)
        {
            m_nonRWFEncoder.EncodeBuffer(value);
            return this;
        }

        /// <summary>
        /// Specifies Set.
        /// </summary>
        /// <param name="value">specifies AnsiPage data using <see cref="EmaBuffer"/></param>
        /// <returns>reference to the current <see cref="OmmAnsiPage"/> object</returns>
        public OmmAnsiPage SetBuffer(EmaBuffer value)
        {
            m_nonRWFEncoder.EncodeBuffer(value);
            return this;
        }

        internal override CodecReturnCode Decode(DecodeIterator dIter)
        {
            if (m_buffer.Decode(dIter) < CodecReturnCode.SUCCESS)
            {
                Code = DataCode.BLANK;
            }
            else
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

            Utilities.AddIndent(m_ToString, indent);
            m_ToString.Append("AnsiPage").AppendLine().Append(EmaUtility.AsHexString(AsHex()));
            Utilities.AddIndent(m_ToString.AppendLine(), indent).Append("AnsiPageEnd").AppendLine();

            return m_ToString.ToString();
        }
    }
}
