/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Globalization;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;

namespace LSEG.Ema.Access;

/// <summary>
/// RmtesBuffer represents RMTES data.
/// </summary>
/// <remarks>
/// RmtesBuffer stores and applies RMTES data.<br/>
/// RmtesBuffer instance contains a copy of the buffer passed on Apply methods.<br/>
/// All methods in this class are single threaded.<br/>
/// The following code snippet shows a simple decoding of RmtesBuffer.<br/>
/// <example>
/// The following code snippet shows applying and displaying an RMTES string:
/// <code>
/// rmtesBuffer.Apply( fe.OmmRmtesValue() );
/// Console.WriteLine(rmtesBuffer.ToString());
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="EmaBuffer"/>
/// <seealso cref="EmaBufferU16"/>
/// <seealso cref="OmmInvalidUsageException"/>
public sealed class RmtesBuffer
{
    #region Constructors

    /// <summary>
    /// Specifies default internal buffer initial size.
    /// </summary>
    /// <seealso cref="RmtesBuffer()"/>
    /// <seealso cref="RmtesBuffer(int)"/>
    public const int RMTES_DECODE_BUFFER_INIT_SIZE = 256;

    /// <summary>
    /// Constructs RmtesBuffer with the default internal buffer initial size.
    /// </summary>
    /// <seealso cref="RMTES_DECODE_BUFFER_INIT_SIZE"/>
    public RmtesBuffer() : this(RMTES_DECODE_BUFFER_INIT_SIZE)
    {
    }

    /// <summary>
    /// Constructs new RmtesBuffer with preallocated memory.
    /// </summary>
    /// <remarks>
    /// Preallocates memory if <paramref name="len"/> is greater than 0
    /// </remarks>
    /// <param name="len">preallocated memory size</param>
    public RmtesBuffer(int len)
    {
        if (len > 0)
        {
            m_RmtesCacheBuffer.Data = new ByteBuffer(len + 20);
            m_RmtesCacheBuffer.AllocatedLength = len + 20;
            m_RmtesCacheBuffer.Length = 0;

            m_Buffer.Data(new ByteBuffer(len));
        }
        else
        {
            m_Buffer.Data(new ByteBuffer(RMTES_DECODE_BUFFER_INIT_SIZE));
        }
    }

    /// <summary>
    /// Assignment constructor.
    /// </summary>
    /// <param name="buf">source buffer</param>
    ///
    /// <seealso cref="RmtesBuffer(byte[],int,int)"/>
    public RmtesBuffer(byte[] buf) : this(buf, 0, buf.Length)
    {
    }


    /// <summary>
    /// Assignment constructor.
    /// </summary>
    ///
    /// <param name="buf">source buffer</param>
    /// <param name="pos">source fragment position</param>
    /// <param name="len">source fragment length</param>
    public RmtesBuffer(byte[] buf, int pos, int len)
    {
        ByteBuffer byteBuffer = new ByteBuffer(len);
        byteBuffer.Put(buf, pos, len).Flip();

        m_Buffer.Data(byteBuffer);

        if (m_Buffer.Length > 0)
        {
            if (m_Decoder.HasPartialRMTESUpdate(m_Buffer))
            {
                throw new OmmInvalidUsageException("Failed to construct RmtesBuffer( byte[] ) due to invalid data.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_DATA);
            }

            m_RmtesCacheBuffer.Data = new ByteBuffer(m_Buffer.Length + 20);
            m_RmtesCacheBuffer.AllocatedLength = m_Buffer.Length + 20;
            m_RmtesCacheBuffer.Length = 0;

            CodecReturnCode retCode = m_Decoder.RMTESApplyToCache(m_Buffer, m_RmtesCacheBuffer);

            while (CodecReturnCode.BUFFER_TOO_SMALL == retCode)
            {
                ReallocateRmtesCacheBuffer();
                retCode = m_Decoder.RMTESApplyToCache(m_Buffer, m_RmtesCacheBuffer);
            }

            if (retCode < CodecReturnCode.SUCCESS)
            {
                throw new
                    OmmInvalidUsageException($"Failed to construct RmtesBuffer(byte[]). Reason: {retCode.GetAsString()}",
                    (int)retCode);
            }

            m_ApplyToCache = true;
        }
    }

    /// <summary>
    /// Copy constructor.
    /// </summary>
    /// <param name="other">source buffer</param>
    public RmtesBuffer(RmtesBuffer other) : this(other.m_Buffer.Length)
    {
        other.m_Buffer.Copy(m_Buffer);

        if (other.m_ApplyToCache && !other.m_DecodedUTF8BufferSet)
        {
            m_RmtesCacheBuffer.AllocatedLength = other.m_RmtesCacheBuffer.AllocatedLength;
            m_RmtesCacheBuffer.Length = other.m_RmtesCacheBuffer.Length;

            if (m_RmtesCacheBuffer.AllocatedLength > 0)
            {
                m_RmtesCacheBuffer.Data = ByteBuffer.Wrap(other.m_RmtesCacheBuffer.Data.Contents);

                m_ApplyToCache = true;
            }
        }
        else if (m_Buffer.Length > 0)
        {
            if (m_Decoder.HasPartialRMTESUpdate(m_Buffer))
            {
                throw new OmmInvalidUsageException("Failed to construct RmtesBuffer(RmtesBuffer) due to invalid data.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_DATA);
            }

            m_RmtesCacheBuffer.Data = new ByteBuffer(m_Buffer.Length + 20);
            m_RmtesCacheBuffer.AllocatedLength = m_Buffer.Length + 20;
            m_RmtesCacheBuffer.Length = 0;

            CodecReturnCode retCode = m_Decoder.RMTESApplyToCache(m_Buffer, m_RmtesCacheBuffer);

            while (CodecReturnCode.BUFFER_TOO_SMALL == retCode)
            {
                ReallocateRmtesCacheBuffer();
                retCode = m_Decoder.RMTESApplyToCache(m_Buffer, m_RmtesCacheBuffer);
            }

            if (retCode < CodecReturnCode.SUCCESS)
            {
                m_RmtesCacheBuffer.Data.Clear();

                throw new OmmInvalidUsageException($"Failed to construct RmtesBuffer(RmtesBuffer). Reason: {retCode.GetAsString()}");
            }

            m_ApplyToCache = true;
        }
    }

    #endregion

    /// <summary>
    /// Returns the content converted as UTF8.
    /// </summary>
    /// <remarks>
    /// NOTE: returned EmaBuffer instance is reused between this method calls and
    /// internally shares byte buffer with this Rmtes instance.
    /// </remarks>
    /// <returns><see cref="EmaBuffer"/> containing RMTES data converted to UTF8</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to convert the data to a UTF8 string.</exception>
    public EmaBuffer GetAsUTF8()
    {
        if (!m_DecodedUTF8BufferSet)
        {
            if (!m_ApplyToCache
                && m_Buffer.Data()?.Contents is not null)
                Apply(m_Buffer);

            if (m_RmtesCacheBuffer.Length == 0)
            {
                return m_EmaUTF8Buffer.Clear();
            }

            if (m_RmtesUTF8Buffer.Data is null)
            {
                m_RmtesUTF8Buffer.Length = m_RmtesCacheBuffer.AllocatedLength;
                m_RmtesUTF8Buffer.Data = new ByteBuffer(m_RmtesUTF8Buffer.Length);
                m_RmtesUTF8Buffer.AllocatedLength = m_RmtesUTF8Buffer.Length;
            }
            else if (m_RmtesCacheBuffer.Length > m_RmtesUTF8Buffer.Length)
            {
                // grow UTF8 buffer to accomodate Cache buffer
                m_RmtesUTF8Buffer.Data.Clear();
                m_RmtesUTF8Buffer.Data.Reserve((m_RmtesUTF8Buffer.Length == 0)
                    ? m_RmtesCacheBuffer.AllocatedLength
                    : m_RmtesCacheBuffer.Length);
                m_RmtesUTF8Buffer.Length = m_RmtesUTF8Buffer.Data.Capacity;
            }

            CodecReturnCode retCode = m_Decoder.RMTESToUTF8(m_RmtesUTF8Buffer, m_RmtesCacheBuffer);

            while (CodecReturnCode.BUFFER_TOO_SMALL == retCode)
            {
                m_RmtesUTF8Buffer.Length += m_RmtesUTF8Buffer.Length;
                m_RmtesUTF8Buffer.Data.Clear();
                m_RmtesUTF8Buffer.Data.Reserve(m_RmtesUTF8Buffer.Length);
                m_RmtesUTF8Buffer.AllocatedLength = m_RmtesUTF8Buffer.Data.Capacity;

                retCode = m_Decoder.RMTESToUTF8(m_RmtesUTF8Buffer, m_RmtesCacheBuffer);
            }

            if (retCode < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException(
                    $"Failed to convert to UTF8 in RmtesBufferImpl.GetAsUTF8(). Reason: {retCode.GetAsString()}");
            }

            m_DecodedUTF8BufferSet = true;
        }

        return m_EmaUTF8Buffer.AssignFrom(m_RmtesUTF8Buffer.Data.Contents, 0, m_RmtesUTF8Buffer.Length);
    }

    /// <summary>
    /// Returns the content converted as UTF16.
    /// </summary>
    /// <remarks>NOTE: returned EmaBufferU16 instance is reused between this method calls.</remarks>
    /// <returns><see cref="EmaBufferU16"/> containing RMTES data converted to UTF16</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to convert the data to a UTF16 string.</exception>
    public EmaBufferU16 GetAsUTF16()
    {
        if (!m_DecodedUTF16BufferSet)
        {
            if (!m_ApplyToCache
                && m_Buffer.Data()?.Contents is not null)
                Apply(m_Buffer);

            if (m_RmtesCacheBuffer.Length == 0)
            {
                return m_EmaUTF16Buffer.Clear();
            }

            if (m_RmtesUTF16Buffer.Data is null)
            {
                m_RmtesUTF16Buffer.Length = m_RmtesCacheBuffer.AllocatedLength;
                m_RmtesUTF16Buffer.Data = new ByteBuffer(m_RmtesUTF16Buffer.Length);
                m_RmtesUTF16Buffer.AllocatedLength = m_RmtesUTF16Buffer.Length;
            }
            else if (m_RmtesCacheBuffer.Length > m_RmtesUTF16Buffer.Length)
            {
                // grow UTF16 buffer to accomodate Cache buffer
                m_RmtesUTF16Buffer.Data.Clear();
                m_RmtesUTF16Buffer.Data.Reserve((m_RmtesUTF16Buffer.Length == 0)
                    ? m_RmtesCacheBuffer.AllocatedLength
                    : m_RmtesCacheBuffer.Length);
                m_RmtesUTF16Buffer.Length = m_RmtesUTF16Buffer.Data.Capacity;
                m_RmtesUTF16Buffer.AllocatedLength = m_RmtesUTF16Buffer.Length;
            }

            CodecReturnCode retCode = m_Decoder.RMTESToUCS2(m_RmtesUTF16Buffer, m_RmtesCacheBuffer);

            while (CodecReturnCode.BUFFER_TOO_SMALL == retCode)
            {
                m_RmtesUTF16Buffer.Length += m_RmtesUTF16Buffer.Length;
                m_RmtesUTF16Buffer.Data.Clear();
                m_RmtesUTF16Buffer.Data.Reserve(m_RmtesUTF16Buffer.Length);
                m_RmtesUTF16Buffer.AllocatedLength = m_RmtesUTF16Buffer.Data.Capacity;

                retCode = m_Decoder.RMTESToUCS2(m_RmtesUTF16Buffer, m_RmtesCacheBuffer);
            }

            if (retCode < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException(
                    $"Failed to convert to UTF16 in RmtesBufferImpl.GetAsUTF8(). Reason: {retCode.GetAsString()}");
            }

            // decode the RMTES byte buffer directly into the EmaBufferU16 avoiding intermittent strings or arrays
            System.Text.Decoder ucsDecoder = Encoding.BigEndianUnicode.GetDecoder();

            int needChars = ucsDecoder.GetCharCount(m_RmtesUTF16Buffer.Data.Contents, 0, m_RmtesUTF16Buffer.Length);
            m_EmaUTF16Buffer.Allocate(needChars);

            ucsDecoder.GetChars(
                new ReadOnlySpan<byte>(m_RmtesUTF16Buffer.Data.Contents, 0, m_RmtesUTF16Buffer.Length),
                m_EmaUTF16Buffer.Buffer, true);

            m_DecodedUTF16BufferSet = true;
        }

        return m_EmaUTF16Buffer;
    }

    /// <summary>
    /// Clears contained content.
    /// </summary>
    /// <returns>Reference to current <see cref="RmtesBuffer"/> object.</returns>
    public RmtesBuffer Clear()
    {
        m_Buffer.Clear();
        m_RmtesCacheBuffer.Clear();

        m_DecodedUTF8BufferSet = false;
        m_RmtesUTF8Buffer.Clear();
        m_EmaUTF8Buffer.Clear();

        m_DecodedUTF16BufferSet = false;
        m_RmtesUTF16Buffer.Clear();
        m_EmaUTF16Buffer.Clear();

        m_ToString = string.Empty;
        m_ToStringSet = false;

        m_ApplyToCache = false;

        return this;
    }


    /// <summary>
    /// Apply passed in RMTES data.
    /// </summary>
    /// <param name="source">specifies RmtesBuffer to be applied to this object</param>
    /// <returns>Reference to current <see cref="RmtesBuffer"/> object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to apply the source data to this RMTES buffer.</exception>
    public RmtesBuffer Apply(RmtesBuffer source)
    {
        if (source is null)
            throw new OmmInvalidUsageException("Source passed in is invalid in Apply(RmtesBuffer source)",
                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

        m_DecodedUTF8BufferSet = false;
        m_DecodedUTF16BufferSet = false;
        m_ToStringSet = false;

        if (m_Buffer.Data() is null)
            m_Buffer.Data(new ByteBuffer(source.m_Buffer.Length));
        else
        {
            ByteBuffer data = m_Buffer.Data();
            data.Clear().Reserve(source.m_Buffer.Length);
            m_Buffer.Data(data);
        }

        source.m_Buffer.Copy(m_Buffer);

        if (source.m_ApplyToCache && !source.m_DecodedUTF8BufferSet)
        {
            // copy data from the source cache buffer and adjust
            m_RmtesCacheBuffer.Data = ByteBuffer.Wrap(source.m_RmtesCacheBuffer.Data.Contents);

            m_RmtesCacheBuffer.Length = source.m_RmtesCacheBuffer.Length;
            m_RmtesCacheBuffer.AllocatedLength = m_RmtesCacheBuffer.Data.Capacity;

            m_ApplyToCache = true;
        }
        else if (m_Buffer.Length > 0)
        {
            if (m_Decoder.HasPartialRMTESUpdate(m_Buffer))
            {
                int actualLen = m_Buffer.Length + m_RmtesCacheBuffer.Length;
                if (actualLen > m_RmtesCacheBuffer.AllocatedLength)
                {
                    m_RmtesCacheBuffer.AllocatedLength += (m_RmtesCacheBuffer.AllocatedLength == 0
                        ? m_Buffer.Length + 20
                        : m_Buffer.Length);
                    ByteBuffer newBuffer = new ByteBuffer(m_RmtesCacheBuffer.AllocatedLength);
                    newBuffer.Put(m_RmtesCacheBuffer.Data.Contents, 0, m_RmtesCacheBuffer.Length);

                    m_RmtesCacheBuffer.Data.Clear();
                    m_RmtesCacheBuffer.Data = newBuffer;
                }
            }
            else
            {
                if (m_Buffer.Length > m_RmtesCacheBuffer.AllocatedLength)
                {
                    m_RmtesCacheBuffer.AllocatedLength += (m_RmtesCacheBuffer.AllocatedLength == 0
                        ? m_Buffer.Length + 20
                        : m_Buffer.Length);

                    m_RmtesCacheBuffer.Data?.Clear();

                    m_RmtesCacheBuffer.Data = new ByteBuffer(m_RmtesCacheBuffer.AllocatedLength);
                }

                m_RmtesCacheBuffer.Length = 0;
            }

            CodecReturnCode retCode = m_Decoder.RMTESApplyToCache(m_Buffer, m_RmtesCacheBuffer);

            while (CodecReturnCode.BUFFER_TOO_SMALL == retCode)
            {
                ReallocateRmtesCacheBuffer();
                retCode = m_Decoder.RMTESApplyToCache(m_Buffer, m_RmtesCacheBuffer);
            }

            if (retCode < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException(
                    $"Failed to apply in RmtesBuffer.Apply(RmtesBuffer). Reason: {retCode.GetAsString()}",
                    (int)retCode);
            }

            m_ApplyToCache = true;
        }
        return this;
    }

    /// <summary>
    /// Apply passed in RMTES data.
    /// </summary>
    /// <param name="source">specifies OmmRmtes containing RMTES data to be applied to this object</param>
    /// <returns>Reference to current <see cref="RmtesBuffer"/> object.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to apply the source data to this RMTES buffer.</exception>
    public RmtesBuffer Apply(OmmRmtes source)
    {
        if (source is null || source.Value is null)
            throw new OmmInvalidUsageException("Source passed in is invalid in Apply(OmmRmtes source)",
                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

        return Apply(source.Value);
    }

    /// <summary>
    /// Apply passed in RMTES data.
    /// </summary>
    /// <param name="buf">specifies byte array containing RMTES data to be applied to this object</param>
    /// <returns>Reference to current <see cref="RmtesBuffer"/> object.</returns>
    public RmtesBuffer Apply(byte[] buf)
    {
        m_DecodedUTF8BufferSet = false;
        m_DecodedUTF16BufferSet = false;
        m_ToStringSet = false;

        m_Buffer.Data(ByteBuffer.Wrap(buf));
        m_Buffer.Data().Flip();

        return Apply(m_Buffer);
    }

    /// <summary>
    /// Returns a string representation of decoded RMTES data.
    /// </summary>
    ///
    /// <exception cref="OmmInvalidUsageException">
    /// if fails to convert
    /// </exception>
    ///
    /// <returns>String containing decoded RMTES data</returns>
    public override string ToString()
    {
        if (!m_DecodedUTF16BufferSet)
        {
            GetAsUTF16();
        }

        if (!m_ToStringSet)
        {
            m_ToString = new string(m_EmaUTF16Buffer.Buffer);
            m_ToStringSet = true;
        }

        return m_ToString;
    }

    #region Implementation details

    internal Eta.Codec.Buffer m_Buffer = new Eta.Codec.Buffer();
    internal bool m_ApplyToCache { get; private set; } = false;

    private RmtesDecoder m_Decoder = new RmtesDecoder();
    private RmtesCacheBuffer m_RmtesCacheBuffer = new RmtesCacheBuffer(0, null, 0);

    private bool m_DecodedUTF16BufferSet = false;
    private Eta.Codec.RmtesBuffer m_RmtesUTF16Buffer = new Eta.Codec.RmtesBuffer(0, null, 0);
    private EmaBufferU16 m_EmaUTF16Buffer = new EmaBufferU16();

    private bool m_DecodedUTF8BufferSet = false;
    private Eta.Codec.RmtesBuffer m_RmtesUTF8Buffer = new Eta.Codec.RmtesBuffer(0, null, 0);
    private EmaBuffer m_EmaUTF8Buffer = new EmaBuffer();

    private bool m_ToStringSet = false;
    private string m_ToString = string.Empty;


    private void ReallocateRmtesCacheBuffer()
    {
        m_RmtesCacheBuffer.Data.Clear();

        m_RmtesCacheBuffer.AllocatedLength += m_RmtesCacheBuffer.AllocatedLength;
        m_RmtesCacheBuffer.Data.Reserve(m_RmtesCacheBuffer.AllocatedLength);

        m_RmtesCacheBuffer.Length = 0;
    }

    private RmtesBuffer Apply(Eta.Codec.Buffer buffer)
    {
        if (buffer.Length > 0)
        {
            if (m_Decoder.HasPartialRMTESUpdate(buffer))
            {
                int actualLen = buffer.Length + m_RmtesCacheBuffer.Length;
                if (actualLen > m_RmtesCacheBuffer.AllocatedLength)
                {
                    int origCacheDataLen = m_RmtesCacheBuffer.Length;

                    m_RmtesCacheBuffer.AllocatedLength +=
                        (m_RmtesCacheBuffer.AllocatedLength == 0
                         ? buffer.Length + 20
                         : buffer.Length);

                    ByteBuffer newByteBuffer = new ByteBuffer(actualLen);
                    newByteBuffer.Put(m_RmtesCacheBuffer.Data.Contents, 0, m_RmtesCacheBuffer.Length);

                    m_RmtesCacheBuffer.Data = newByteBuffer;
                    m_RmtesCacheBuffer.Length = origCacheDataLen;
                    m_RmtesCacheBuffer.AllocatedLength = actualLen;
                }
            }
            else
            {
                if (buffer.Length > m_RmtesCacheBuffer.AllocatedLength)
                {
                    m_RmtesCacheBuffer.AllocatedLength +=
                        (m_RmtesCacheBuffer.AllocatedLength == 0 ? buffer.Length + 20 : buffer.Length);

                    m_RmtesCacheBuffer.Data = new ByteBuffer(m_RmtesCacheBuffer.AllocatedLength);
                }

                m_RmtesCacheBuffer.Length = 0;
            }

            CodecReturnCode retCode = m_Decoder.RMTESApplyToCache(buffer, m_RmtesCacheBuffer);

            while (CodecReturnCode.BUFFER_TOO_SMALL == retCode)
            {
                m_RmtesCacheBuffer.AllocatedLength += m_RmtesCacheBuffer.AllocatedLength;
                m_RmtesCacheBuffer.Data.Clear();
                m_RmtesCacheBuffer.Data.Reserve(m_RmtesCacheBuffer.AllocatedLength);
                m_RmtesCacheBuffer.Length = 0;

                retCode = m_Decoder.RMTESApplyToCache(buffer, m_RmtesCacheBuffer);
            }

            if (retCode < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to RmtesBuffer.Apply(byte[]). Reason: {retCode.GetAsString()}");
            }

            m_ApplyToCache = true;
        }

        return this;
    }

    #endregion
}
