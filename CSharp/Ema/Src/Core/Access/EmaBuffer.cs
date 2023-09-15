/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;
using LSEG.Eta.Common;

namespace LSEG.Ema.Access;

/// <summary>
/// EmaBuffer represents a general use binary buffer.
/// </summary>
/// <remarks>
/// <para>
/// EmaBuffer is a buffer of 8 bit long characters where each character is represented by
/// char or byte.
/// </para>
/// <para>
/// EmaBuffer class contains a copy of the passed in buffer.
/// </para>
/// <para>
/// All methods in this class are SingleThreaded.
/// </para>
/// </remarks>
///
public sealed class EmaBuffer : IEquatable<EmaBuffer>
{

    #region Constructors

    /// <summary>
    /// Constructs EmaBuffer.
    /// </summary>
    public EmaBuffer()
    {
        m_Buffer = Array.Empty<byte>();
        m_Length = 0;
        m_Offset = 0;
    }

    /// <summary>
    /// Copy constructor
    /// </summary>
    ///
    /// <exception cref="OutOfMemoryException">
    /// if application runs out of memory
    /// </exception>
    ///
    /// <param name="buf">memory containing copied in buffer</param>
    public EmaBuffer(Span<byte> buf)
    {
        m_Buffer = new byte[buf.Length];
        CopyFrom(buf);
    }

    /// <summary>
    /// Copy constructor.
    /// </summary>
    ///
    /// <exception cref="OutOfMemoryException">
    /// if application runs out of memory
    /// </exception>
    ///
    /// <param name="buf">copied in EmaBuffer object</param>
    public EmaBuffer(EmaBuffer buf)
    {
        m_Buffer = new byte[buf.Length];
        CopyFrom(buf.Buffer);
    }

    /// <summary>
    /// Copy constructor that copies contents of the byte array with the given offset of
    /// the specified length.
    /// </summary>
    ///
    /// <exception cref="OutOfMemoryException">
    /// if application runs out of memory
    /// </exception>
    ///
    /// <param name="buf"> source buffer</param>
    /// <param name="offset"> source data offset within the buffer</param>
    /// <param name="len"> length of source data block</param>
    public EmaBuffer(byte[] buf, int offset, int len)
    {
        m_Buffer = new byte[len];
        CopyFrom(buf, offset, len);
    }

    #endregion

    #region Operations

    /// <summary>
    /// Clears contained buffer.
    /// </summary>
    ///
    /// <returns>reference to this object</returns>
    public EmaBuffer Clear()
    {
        m_Length = 0;
        m_Offset = 0;
        m_Buffer = Array.Empty<byte>();

        return this;
    }

    /// <summary>
    /// Method to copy EmaBuffer contents from another EmaBuffer.
    /// </summary>
    ///
    /// <param name="buf">span containing copied in buffer</param>
    ///
    /// <returns>reference to this object</returns>
    public EmaBuffer CopyFrom(Span<byte> buf)
    {
        if (buf.Length > m_Offset + m_Buffer.Length)
            Array.Resize(ref m_Buffer, m_Offset + buf.Length);

        buf.CopyTo(new Span<byte>(m_Buffer, m_Offset, buf.Length));
        m_Length = buf.Length;

        return this;
    }

    /// <summary>
    /// Method to set Buffer.
    /// </summary>
    ///
    /// <exception cref="OutOfMemoryException">
    /// if application runs out of memory
    /// </exception>
    ///
    /// <param name="buf">source buffer</param>
    /// <param name="offset">data offset in the source buffer</param>
    /// <param name="len">source data length</param>
    ///
    /// <returns>
    /// reference to this object
    /// </returns>
    public EmaBuffer CopyFrom(byte[] buf, int offset, int len)
    {
        if (len > m_Offset + m_Buffer.Length)
            Array.Resize(ref m_Buffer, m_Offset + len);

        Array.Copy(buf, offset, m_Buffer, m_Offset, len);
        m_Length = len;

        return this;
    }

    /// <summary>
    /// Method to append this object with the passed in EmaBuffer object
    /// </summary>
    ///
    /// <exception cref="OutOfMemoryException">
    /// if application runs out of memory
    /// </exception>
    ///
    /// <param name="buf">EmaBuffer to append to this object</param>
    ///
    /// <returns> reference to this object</returns>
    public EmaBuffer Append(EmaBuffer buf)
    {
        Append(buf.Buffer);
        return this;
    }

    /// <summary>
    /// Method to append this object with the passed in char
    /// </summary>
    ///
    /// <exception cref="OutOfMemoryException">
    /// if application runs out of memory
    /// </exception>
    ///
    /// <param name="c"> character to append to this object</param>
    ///
    /// <returns>reference to this object</returns>
    public EmaBuffer Append(byte c)
    {
        if (m_Length == m_Buffer.Length)
            Array.Resize(ref m_Buffer, m_Offset + m_Length + 1);

        m_Buffer[m_Offset + m_Length] = c;
        m_Length++;

        return this;
    }

    /// <summary>
    /// Method to append this object with the passed in char buffer
    /// </summary>
    ///
    /// <exception cref="OutOfMemoryException">
    /// if application runs out of memory
    /// </exception>
    ///
    /// <param name="buf"> memory containing appended buffer</param>
    ///
    /// <returns> reference to this object</returns>
    public EmaBuffer Append(Span<byte> buf)
    {
        // ensure capacity for new data
        if (m_Buffer.Length < m_Offset + m_Length + buf.Length)
        {
            Array.Resize(ref m_Buffer, m_Offset + m_Length + buf.Length);
        }

        buf.CopyTo(new Span<byte>(m_Buffer, m_Offset + m_Length, buf.Length));
        m_Length += buf.Length;

        return this;
    }

    #endregion

    #region Accessors

    /// <summary>
    /// Internal storage memory.
    /// </summary>
    ///
    /// <value>
    /// Span pointing to data within internal buffer.
    /// </value>
    ///
    /// <seealso cref="Contents"/>
    /// <seealso cref="AsByteArray()"/>
    public Span<byte> Buffer { get => new Span<byte>(m_Buffer, m_Offset, m_Length); }

    /// <summary>
    /// Returns length of the internal storage memory.
    /// </summary>
    ///
    /// <value>buffer length inside internal buffer</value>
    ///
    /// <seealso cref="Contents"/>
    /// <seealso cref="Offset"/>
    public int Length { get => m_Length; }

    /// <summary>
    /// Data block offset inside internal memory buffer.
    /// </summary>
    ///
    /// <seealso cref="Contents"/>
    /// <seealso cref="Length"/>
    public int Offset { get => m_Offset; }

    /// <summary>
    /// Returns a hexadecimal string representation of this buffer contents suitable for
    /// screen output.
    /// </summary>
    ///
    /// <remarks>
    /// Unlike <see cref="AsHexString()"/> lines of hex numbers are not appended with
    /// string representation of processed bytes.
    /// </remarks>
    ///
    /// <returns> string used for printing out content of the internal buffer to
    ///   screen</returns>
    ///
    /// <seealso cref="AsHexString()"/>
    public string AsRawHexString()
    {
        StringBuilder currentLine = new StringBuilder();
        StringBuilder all = new StringBuilder();

        int currentChar = 0;
        bool eobyte = false;

        for (int i = m_Offset; i < m_Length; i++)
        {
            if (!(currentChar < CHARS_PER_LINE))
            {
                all.Append('\n');

                currentChar = 0;
                eobyte = false;
            }

            byte b = this[i];
            all.Append(b.ToString("X2"));
            if (eobyte
                && i != m_Length - 1
                && currentChar != CHARS_PER_LINE - 1)
                all.Append(' ');

            eobyte = !eobyte;

            ++currentChar;
        }

        return all.ToString();
    }

    /// <summary>
    /// Returns a hexadecimal string representation of this buffer contents suitable for
    /// screen output with string representation of processed bytes.
    /// </summary>
    ///
    /// <returns> string used for printing out content of the internal buffer to screen</returns>
    ///
    /// <seealso cref="AsHexString()"/>
    public string AsHexString()
    {
        // (2 chars per byte) + (1 char for representation) + (half as many spaces)
        // + (1 space after char 7) + (2 spaces between hex and print)
        int lineWidth = CHARS_PER_LINE * 3 + CHARS_PER_LINE / 2 + 3;
        // where print representations start
        int printOffset = CHARS_PER_LINE * 2 + CHARS_PER_LINE / 2 + 3;

        // preallocate sufficient space for the complete output
        StringBuilder all = new StringBuilder(lineWidth + lineWidth * Length / CHARS_PER_LINE);

        char[] currentLine = new char[lineWidth];
        Array.Fill(currentLine, ' ');

        int currentChar = 0;
        bool eobyte = false;
        int pos = 0;

        for (int posInBuff = m_Offset; posInBuff < m_Length; posInBuff++)
        {
            if (!(currentChar < CHARS_PER_LINE))
            {
                // previous line is complete
                all.Append(new string(currentLine)).Append('\n');

                // reset for the next line
                Array.Fill(currentLine, ' ');

                pos = 0;
                currentChar = 0;
                eobyte = false;
            }

            byte thisByte = this[posInBuff];
            thisByte.TryFormat(currentLine.AsSpan(pos), out _, "X2");

            pos += eobyte ? 3 : 2;
            if (currentChar == 7)
                pos += 1;

            eobyte = !eobyte;

            // print representation for printable characters, '.' for the rest
            if (thisByte > 31 && thisByte < 127)
                currentLine[printOffset + currentChar] = (char)thisByte;
            else
                currentLine[printOffset + currentChar] = '.';

            ++currentChar;
        }

        // handle incomplete line
        if (currentChar > 0)
            all.Append(new string(currentLine, 0, printOffset + currentChar));

        return all.ToString();
    }

    /// <summary>
    /// Returns a new byte array with this EmaBuffer contents.
    /// </summary>
    ///
    /// <exception cref="OutOfMemoryException">
    /// if application runs out of memory
    /// </exception>
    ///
    /// <returns>this EmaBuffer contents as a new byte array with the <see cref="Length"/>
    ///   size </returns>
    ///
    /// <seealso cref="Contents"/>
    /// <seealso cref="Buffer"/>
    public byte[] AsByteArray()
    {
        return Buffer.ToArray();
    }

    /// <summary>
    /// Provides direct access to the underlying buffer.
    /// </summary>
    ///
    /// <remarks>
    /// The data block within underlying buffer starts from <see cref="Offset"/> and is
    /// <see cref="Length"/> bytes long.
    /// </remarks>
    ///
    /// <seealso cref="AsByteArray()"/>
    /// <seealso cref="Buffer"/>
    /// <seealso cref="Offset"/>
    /// <seealso cref="Length"/>
    public byte[] Contents { get => m_Buffer; }

    /// <summary>
    /// Equality operator.
    /// </summary>
    ///
    /// <param name="buf"> compared EmaBuffer object</param>
    ///
    /// <returns><c>true</c> if this and passed in object contents match</returns>
    public bool Equals(EmaBuffer? buf)
    {
        if (buf is null)
            return false;

        if (buf.Length == m_Length)
        {
            // same length, same array
            if (buf.m_Offset == m_Offset
                && ReferenceEquals(buf.m_Buffer, m_Buffer))
                return true;

            // same length, different arrays
            for (int i = 0; i < m_Length; i++)
            {
                if (buf[i] != this[i])
                    return false;
            }
            return true;
        }
        return false;
    }

    /// <summary>
    /// Read-write index operator.
    /// </summary>
    ///
    /// <exception cref="ArgumentOutOfRangeException">
    /// if passed in index is outside of the contained buffer
    /// </exception>
    ///
    /// <param name="index"> specifies position to read/write</param>
    /// <value>byte at the specified position</value>
    public byte this[int index]
    {
        get => m_Buffer[m_Offset + index];
        set
        {
            m_Buffer[m_Offset + index] = value;
        }
    }

    #endregion

    #region Private members

    internal byte[] m_Buffer;
    // Data block length
    private int m_Length;
    // Data block offset within m_Buffer
    private int m_Offset;

    private const int CHARS_PER_LINE = 16;

    internal void CopyTo(ByteBuffer destBuffer)
    {
        destBuffer.Put(m_Buffer, m_Offset, m_Length);
    }

    // Turns EmaBuffer into a span-like window over the provided buffer. This method is
    // intended to be used from within the EMA library to avoid memory copy.
    internal EmaBuffer AssignFrom(byte[] buf, int offset, int len)
    {
        m_Buffer = buf;
        m_Length = len;
        m_Offset = offset;

        return this;
    }

    #endregion
}
