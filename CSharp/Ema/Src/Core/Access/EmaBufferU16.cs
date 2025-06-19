/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;

namespace LSEG.Ema.Access;

/// <summary>
/// EmaBufferU16 represents general use character buffer.
/// </summary>
/// <remarks>
/// <para>
/// EmaBufferU16 is a buffer of 16 bit long characters where each character is represented
/// by <see chref="char"/>.</para>
/// <para>
/// EmaBufferU16 is used to contain UTF16 data.</para>
/// <para>
/// EmaBufferU16 class contains a copy of the buffer passed on set methods.</para>
/// <para>
/// All methods in this class are single threaded.</para>
/// </remarks>
public sealed class EmaBufferU16 : IEquatable<EmaBufferU16>, IComparable<EmaBufferU16>
{
    #region Constructor

    /// <summary>
    /// Constructs empty EmaBufferU16
    /// </summary>
    public EmaBufferU16()
    {
        m_Length = 0;
        m_Buffer = Array.Empty<char>();
    }

    /// <summary>
    /// Assignment constructor
    /// </summary>
    /// <param name="buf"> memory containing copied in buffer</param>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16(Span<char> buf)
    {
        m_Buffer = new char[buf.Length];
        CopyFrom(buf);
    }

    /// <summary>
    /// Copy constructor.
    /// </summary>
    /// <param name="buf">copied in EmaBufferU16 object</param>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16(EmaBufferU16 buf)
    {
        m_Buffer = new char[buf.Length];
        CopyFrom(buf.Buffer);
    }

    /// <summary>
    /// Copy constructor that copies contents of the byte array with the given offset of
    /// the specified length.
    /// </summary>
    /// <param name="buf">copied in EmaBufferU16 object</param>
    /// <param name="offset"> source data offset within the buffer</param>
    /// <param name="len"> length of source data block</param>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16(char[] buf, int offset, int len)
    {
        m_Buffer = new char[len];
        CopyFrom(buf, offset, len);
    }

    /// <summary>
    /// Construct empty buffer with preallocated memory.
    /// </summary>
    /// <param name="len">preallocated memory size</param>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16(int len)
    {
        m_Buffer = new char[(int)len];
    }

    #endregion

    #region Operations

    /// <summary>
    /// Clears contained buffer.
    /// </summary>
    /// <returns>Reference to current <see cref="EmaBufferU16"/> object.</returns>
    public EmaBufferU16 Clear()
    {
        m_Length = 0;
        return this;
    }

    /// <summary>
    /// Method to set Buffer.
    /// </summary>
    /// <param name="buf"> memory containing copied in buffer</param>
    /// <returns>Reference to current <see cref="EmaBufferU16"/> object.</returns>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16 CopyFrom(Span<char> buf)
    {
        Allocate(buf.Length);

        buf.CopyTo(m_Buffer);

        return this;
    }

    /// <summary>
    /// Method to set Buffer.
    /// </summary>
    /// <param name="buf"> memory containing copied in buffer</param>
    /// <param name="offset"> source data offset within the buffer</param>
    /// <param name="len"> length of source data block</param>
    /// <returns>Reference to current <see cref="EmaBufferU16"/> object.</returns>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16 CopyFrom(char[] buf, int offset, int len)
    {
        Allocate(len);

        Array.Copy(buf, offset, m_Buffer, 0, len);

        return this;
    }

    /// <summary>
    /// Method to append this object with the passed in EmaBufferU16 object
    /// </summary>
    /// <param name="buf">EmaBufferU16 to append to this object</param>
    /// <returns>Reference to current <see cref="EmaBufferU16"/> object.</returns>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16 Append(EmaBufferU16 buf)
    {
        Append(buf.Buffer);
        return this;
    }

    /// <summary>
    /// Appends this object with the passed in 16 byte long character
    /// </summary>
    /// <param name="c">character to append to this object</param>
    /// <returns>Reference to current <see cref="EmaBufferU16"/> object.</returns>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16 Append(char c)
    {
        int pos = m_Length;

        Resize(m_Length + 1);

        m_Buffer[pos] = c;

        return this;
    }

    /// <summary>
    /// Appends this object with the passed in char buffer
    /// </summary>
    /// <param name="buf">span containing appended buffer</param>
    /// <returns>Reference to current <see cref="EmaBufferU16"/> object.</returns>
    /// <exception cref="OutOfMemoryException">Thrown if application runs out of memory</exception>
    public EmaBufferU16 Append(Span<char> buf)
    {
        int pos = m_Length;

        Resize(m_Length + buf.Length);

        buf.CopyTo(new Span<char>(m_Buffer, pos, buf.Length));

        return this;
    }

    /// <summary>
    /// Read-write index operator.
    /// </summary>
    /// <param name="index"> specifies position to read/write</param>
    /// <value>byte at the specified position</value>
    /// <exception cref="ArgumentOutOfRangeException">Thrown if passed in index is outside of the contained buffer</exception>
    public char this[int index]
    {
        get => m_Buffer[index];
        set => m_Buffer[index] = value;
    }
    #endregion

    #region Accessors

    /// <summary>
    /// Returns pointer to the internal storage memory.
    /// </summary>
    /// <value>pointer to the internal memory area containing buffer data</value>
    /// <seealso cref="Contents"/>
    public Span<char> Buffer { get => new Span<char>(m_Buffer, 0, m_Length); }

    /// <summary>
    /// Provides direct access to the underlying buffer.
    /// </summary>
    /// <remarks>
    /// The data block within underlying buffer starts from 0 and is <see cref="Length"/> characters long.
    /// </remarks>
    /// <seealso cref="Buffer"/>
    public char[] Contents { get => m_Buffer; }

    /// <summary>
    /// Returns length of the internal storage memory.
    /// </summary>
    /// <returns>length of the internal buffer</returns>
    public int Length { get => m_Length; }

    /// <summary>
    /// Compare operator.
    /// </summary>
    /// <param name="buf">compared EmaBufferU16 object</param>
    /// <returns><c>true</c> if this and passed in object match</returns>
    public bool Equals(EmaBufferU16? buf)
    {
        if (buf is null)
            return false;

        if (buf.Length == Length)
        {
            // same length, same array
            if (ReferenceEquals(buf.m_Buffer, m_Buffer))
                return true;

            // same length, different arrays
            for (int i = 0; i < Length; i++)
            {
                if (buf.Buffer[i] != m_Buffer[i])
                    return false;
            }
            return true;
        }

        return false;
    }

    
    /// <summary>
    /// Implements the CompareTo method for the IComparable interface.
    /// </summary>
    /// <param name="other">Other <see cref="EmaBufferU16"/> to be compared to</param>
    /// <returns>Integer indicating if the current object should preceed, have the same position, or follow the compared other buffer</returns>
    public int CompareTo(EmaBufferU16? other)
    {
        if (other is null)
            return 1;

        if (ReferenceEquals(this, other))
            return 0;

        int len = this.Length > other.Length ? other.Length : Length;

        for (int i = 0; i < len; i++)
        {
            int cmp = this[i].CompareTo(other[i]);
            if (cmp != 0)
                return cmp;
        }
        return this.Length.CompareTo(other.Length);
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="EmaBufferU16"/> object.</returns>
    public override string ToString()
    {
        return new String(m_Buffer);
    }

    #endregion

    #region Private members

    private char[] m_Buffer;
    private int m_Length;

    internal EmaBufferU16 AssignFrom(char[] buf, int len)
    {
        m_Buffer = buf;
        m_Length = len;
        return this;
    }

    // if needed, expands m_Buffer to len entries, existing data is preserved
    internal void Resize(int len)
    {
        if (m_Buffer.Length < len)
            Array.Resize(ref m_Buffer, len);

        m_Length = len;
    }

    // if needed, replaces m_Buffer with a new buffer of len entries, existing data is discarded
    internal void Allocate(int len)
    {
        if (m_Buffer.Length < len)
            m_Buffer = new char[len];

        m_Length = len;
    }

    #endregion
}
