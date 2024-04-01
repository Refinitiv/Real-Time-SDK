/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmOpaque represents Opaque binary data format in Omm.
    /// </summary>
    /// <remarks>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and extracting of Opaque and its content.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// </remarks>
    public sealed class OmmOpaque : ComplexType
    {
        private OmmBuffer m_buffer = new OmmBuffer();
        private OmmNonRwfEncoder m_nonRWFEncoder;

        /// <summary>
        /// Constructor for OmmOpaque class
        /// </summary>
        public OmmOpaque()
        {
            m_nonRWFEncoder = new OmmNonRwfEncoder();
            Encoder = m_nonRWFEncoder;
            Encoder.m_encoderOwner = this;
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            DecodePrimitiveType = DecodeOmmOpaquePrimitive;
            DecodeComplexType = DecodeOmmOpaqueComplex;
            m_dataType = Access.DataType.DataTypes.OPAQUE;
        }

        /// <summary>
        /// Returns contained XML buffer.
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing the Opaque data</returns>
        public EmaBuffer Buffer { get => m_buffer.Value; }

        /// <summary>
        /// Returns a string representation of the class instance.
        /// </summary>
        /// <returns>string representation of the class object.</returns>
        public string? GetString()
        { return m_buffer!.Value!.ToString(); }

        /// <summary>
        /// Clears the OmmOpaque.
        /// Invoking Clear() method clears all the values and resets all the defaults.
        /// </summary>
        /// <returns>Reference to the current <see cref="OmmOpaque"/> object.</returns>
        public OmmOpaque Clear()
        {
            Clear_All();
            return this;
        }

        /// <summary>
        /// Specifies Set.
        /// </summary>
        /// <param name="value">specifies Opaque data using <see cref="EmaBuffer"/></param>
        /// <returns>Reference to the current <see cref="OmmOpaque"/> object.</returns>
        public OmmOpaque SetBuffer(EmaBuffer value)
        {
            m_nonRWFEncoder.EncodeBuffer(value);
            return this;
        }

        /// <summary>
        /// Specifies Set.
        /// </summary>
        /// <param name="value">specifies Opaque data using SetString</param>
        /// <returns>Reference to the current <see cref="OmmOpaque"/> object.</returns>
        public OmmOpaque SetString(string value)
        {
            m_nonRWFEncoder.EncodeBuffer(value);
            return this;
        }

        internal CodecReturnCode DecodeOmmOpaquePrimitive(DecodeIterator dIter)
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

        internal CodecReturnCode DecodeOmmOpaqueComplex(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
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
            m_ToString.Append("Opaque\n").Append(EmaUtility.AsHexString(AsHex()));
            Utilities.AddIndent(m_ToString.Append("\n"), indent).Append("OpaqueEnd\n");

            return m_ToString.ToString();
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="OmmOpaque"/> object.</returns>
        public override string ToString()
        {
            return ToString(0);
        }

        internal override string FillString(int indent)
        {
            throw new System.NotImplementedException();
        }
    }
}