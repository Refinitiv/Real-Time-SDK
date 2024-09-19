/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Runtime.CompilerServices;
using System.Text;
using LSEG.Eta.Common;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// ETA Buffer represents some type of user-provided content along with the
	/// content's length.
	/// </summary>
	/// <remarks>
	/// 
	/// <para>
	/// Used by data encoder/decoder and message packages. The <c>Buffer</c>
	/// has a position and length. When accessing the backing data, use
	/// <see cref="Buffer.Position"/> for the position and <see cref="Buffer.Length"/> for
	/// the length, not the position and limit of the <see cref="ByteBuffer"/> returned
	/// from <see cref="Buffer.Data()"/>. Blank buffers are conveyed as a <see cref="Buffer.Length"/>
	/// of 0.
	/// 
	/// </para>
	/// <para>
	/// <see cref="Buffer"/> can:
	/// <list type="bullet">
	/// <item>
	/// Represent various buffer and string types, such as ASCII, RMTES, or UTF8
	/// strings.</item>
	/// <item>
	/// Contain or reference encoded data on both container and message header
	/// structures.</item>
	/// </list>
	/// </para>
	/// </remarks>
	sealed public class Buffer
    {
        /* The length of the buffer. */
        private int _length;
        /* The position of the data. */
        internal int _position;
        /* The actual data represented as a ByteBuffer. */
        internal ByteBuffer _data;
        /* The actual data represented as a String. */
        private string _dataString;

        internal bool _isBlank;

        private Encoding m_Encoding;

        /// <summary>
        /// Creates <see cref="Buffer"/> with no data.
        /// must be called later for buffer to be useful.
        /// </summary>
        /// <seealso cref="Buffer"/>
        public Buffer()
        {
            m_Encoding = Encoding.ASCII;
        }

        /// <summary>
		/// Sets the Buffer data to the ByteBuffer. This buffer's position and length
		/// will be taken from the specified ByteBuffer.
		/// </summary>
		/// <param name="data"> the <see cref="ByteBuffer"/> data to set.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success, or
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if data is <c>null</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Data(ByteBuffer data)
        {
            if (data != null)
            {
                Data_internal(data);
                return CodecReturnCode.SUCCESS;
            }
            else
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Data_internal(ByteBuffer data)
        {
            _data = data;
            _dataString = null;
            _position = data.BufferPosition();
            _length = data.BufferLimit() - _position;
            _isBlank = false;
        }

        /// <summary>
        /// Sets the Buffer data to the ByteBuffer. This buffer's position and length
        /// will be set to the specified position and length.
        /// </summary>
        /// <param name="data"> the <see cref="ByteBuffer"/> to set. </param>
        /// <param name="position"> the data's starting position. </param>
        /// <param name="length"> the data's length.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
        ///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if data is <c>null</c>, or if
        ///         position or length is outside of the data's capacity.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Data(ByteBuffer data, int position, int length)
        {
            if (data != null)
            {
                if (position >= 0 && position <= data.BufferLimit() && length >= 0 && (position + length) <= data.BufferLimit())
                {
                    Data_internal(data, position, length);
                    return CodecReturnCode.SUCCESS;
                }
                else
                {
                    return CodecReturnCode.INVALID_ARGUMENT;
                }
            }
            else
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Data_internal(ByteBuffer data, int position, int length)
        {
            _data = data;
            _dataString = null;
            _position = position;
            _length = length;
            _isBlank = false;
        }

        /// <summary>
        /// Copies this Buffer's data starting at this Buffer's position, for this
        /// Buffer's length, into the destBuffer starting at the destBuffer's position.
        /// </summary>
        /// <param name="destBuffer"> A <see cref="ByteBuffer"/> large enough
        ///            to hold the contents of this <see cref="Buffer"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
        ///   <c>CodecReturnCode.INVALID_ARGUMENT</c> if the <paramref name="destBuffer"/> is
        ///   <c>null</c>, or <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the
        ///   <paramref name="destBuffer"/> is too small.
        /// </returns>
        /// <seealso cref="Buffer.Length"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Copy(ByteBuffer destBuffer)
        {
            if (destBuffer is null)
                return CodecReturnCode.INVALID_ARGUMENT;

            if (Length == 0)
                return CodecReturnCode.SUCCESS;

            if (Length > destBuffer.Remaining)
                return CodecReturnCode.BUFFER_TOO_SMALL;

            int length = GetLength();

            if (_dataString is null)
            {
                // ByteBuffer to ByteBuffer.
                destBuffer.Put(_data._data, _position, length);
            }
            else
            {
                // no need to check return code since we did range checking already.
                CopyStringToByteBuffer(_dataString, destBuffer);
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode Copy(byte[] destBuffer)
        {
            return Copy(destBuffer, 0);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode Copy(byte[] destBuffer, int destOffset)
        {
            int length = GetLength();
            if (length != 0)
            {
                if (destBuffer != null)
                {
                    if (length <= (destBuffer.Length - destOffset))
                    {
                        if (_dataString is null)
                        {
                            // ByteBuffer to byte[].
                            _data.ReadBytesInto(destBuffer, destOffset, _position + length);
                        }
                        else
                        {
                            // no need to check return code since we did range checking already.
                            CopyStringToByteArray(_dataString, destBuffer, destOffset);
                        }
                    }
                    else
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                }
                else
                {
                    return CodecReturnCode.INVALID_ARGUMENT;
                }
            }
            else
            {
                // length is zero, no copy needed for blank
                return CodecReturnCode.SUCCESS;
            }

            return 0; // success
        }

        /// <summary>
		/// Copies this Buffer's data starting at this Buffer's position, for this
		/// Buffer's length, into the destBuffer starting at the destBuffer's
		/// position.
		/// </summary>
		/// <param name="destBuffer"> A <see cref="Buffer"/> backed by a
		///            <see cref="ByteBuffer"/> large enough to hold
		///            the contents of this <see cref="Buffer"/>.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///   <c>CodecReturnCode.INVALID_ARGUMENT</c> if the <paramref name="destBuffer"/> is <c>null</c>,
		///   the backing buffer is <c>null</c> or the backing buffer is a String,
		///   or <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the <paramref name="destBuffer"/> is too small.
		/// </returns>
		/// <seealso cref="Buffer.Length"/>
		/// <seealso cref="Buffer.Position"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Copy(Buffer destBuffer)
        {
            int length = GetLength();
            if (length != 0)
            {
                if (destBuffer != null)
                {
                    if ((destBuffer).DataString() is null)
                    {
                        ByteBuffer destByteBuffer = destBuffer.Data();
                        if (destByteBuffer != null)
                        {
                            if (length <= destBuffer.Capacity)
                            {
                                destByteBuffer.WritePosition = destBuffer.Position;

                                if (_dataString is null)
                                {
                                    // ByteBuffer to ByteBuffer.
                                    destByteBuffer.Put(_data.Contents, _position, length);
                                }
                                else
                                {
                                    // no need to check return code since we did range checking already.
                                    CopyStringToByteBuffer(_dataString, destByteBuffer);
                                }

                                return CodecReturnCode.SUCCESS; // success
                            }
                            else
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                        }
                        else
                        {
                            return CodecReturnCode.INVALID_ARGUMENT;
                        }
                    }
                    else
                    {
                        return CodecReturnCode.INVALID_ARGUMENT;
                    }
                }
                else
                {
                    return CodecReturnCode.INVALID_ARGUMENT;
                }
            }
            else
            {
                // length is zero, no copy needed for blank
                return CodecReturnCode.SUCCESS;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode CopyWithOrWithoutByteBuffer(Buffer buffer)
        {
            if (_data != null)
            {
                if (buffer._data == null || Length > buffer.Length)
                {
                    ByteBuffer newByteBuffer = new ByteBuffer(Length);
                    buffer.Data(newByteBuffer);
                }
            }
            else if (_dataString != null)
            {
                buffer.Data_internal(_dataString);
                return CodecReturnCode.SUCCESS;
            }
            return Copy(buffer);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ByteBuffer Copy()
        {
            return ByteBuffer.Wrap(DataBytes());
        }

        /// <summary>
		/// The backing ByteBuffer is initially set along with initial position and length.
        /// </summary>
        /// 
        /// <remarks>
		/// <para>This method returns the initial length if there was no operation on the backing 
		/// ByteBuffer that would change the position (such as get or put).</para>
		/// 
        /// <para>If the backing ByteBuffer position has been changed by reading or writing to 
		/// the buffer, this method returns the change in position, i.e. difference between 
		/// current position and initial position.</para>
		/// </remarks>
		/// <value> the length </value>
		public int Length
        {
            get
            {
                int len = _length;

                if (_data != null && _data.BufferPosition() > _position)
                {
                    if (_data.BufferPosition() - _position < len)
                    {
                        len = _data.BufferPosition() - _position;
                    }
                }

                return len;
            }
        }

        /// <summary>
        /// Gets the length of the data in the ByteBuffer
        /// </summary>
        /// <returns>The length of the ByteBuffer</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int GetLength()
        {
            int len = _length;

            if (_data != null && _data.BufferPosition() > _position)
            {
                if (_data.BufferPosition() - _position < len)
                {
                    len = _data.BufferPosition() - _position;
                }
            }

            return len;
        }

        /// <summary>
		/// Returns the initial position of the buffer.
		/// </summary>
		/// <value> the position </value>
		public int Position { get => _position; }

        /// <summary>
        /// Converts the underlying buffer into a String. This should only be called
        /// when the Buffer is known to contain ASCII data. This method creates
        /// garbage unless the underlying buffer is a String.
        /// </summary>
        /// <returns>The String representation</returns>
        public override string ToString()
        {
            string retStr = null;

            if (_data != null)
            {
                retStr = m_Encoding.GetString(DataBytes(), 0, _length);
            }
            else if (!string.ReferenceEquals(_dataString, null))
            {
                retStr = _dataString;
            }

            return retStr;
        }

        /// <summary>
		/// Sets the Buffer data to the contents of the string. This buffer's
		/// position will be set to zero and length will be set to the specified string's length.
		/// </summary>
		/// <param name="str"> the string to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success, or
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if data is <c>null</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Data(string str)
        {
            if (!(str is null))
            {
                Data_internal(str);
                return CodecReturnCode.SUCCESS;
            }
            else
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Data_internal(string str)
        {
            _dataString = str;
            _data = null;
            _position = 0;
            _length = str.Length;
            _isBlank = false;
        }

        /* Appends a byte to the buffer. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void AppendByte(byte b)
        {
            _data.Write(b);
        }

        /// <summary>
		/// Clears the buffer by setting its underlying ByteBuffer to null and its
		/// length and position to 0. If the underlying ByteBuffer is not referenced 
		/// in any other place, it will be a subject to GC.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            _data = null;
            _dataString = null;
            _length = 0;
            _position = 0;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Blank()
        {
            Clear();
            _isBlank = true;
        }

        /// <summary>
        /// Returns <c>true</c> if the Buffer has been decoded as blank.
        /// </summary>
        public bool IsBlank { get => _isBlank; }

        /// <summary>
        /// Return a copy of the Data/DataString.
        /// </summary>
        /// <returns>copy of this buffer.
        /// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private byte[] DataBytes()
        {
            byte[] bufferBytes = null;

            if (_data != null)
            {
                bufferBytes = new byte[_length];
                System.Buffer.BlockCopy(_data.Contents, _position, bufferBytes, 0, _length);
            }
            else if (!string.IsNullOrEmpty(_dataString))
            {
                bufferBytes = System.Text.Encoding.ASCII.GetBytes(_dataString.ToCharArray(), _position, _length);
            }
            else
            {
                bufferBytes = new byte[_length];
            }
            return bufferBytes;
        }

        /* Gets a data byte from a position in the buffer. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal byte DataByte(int position)
        {
            byte retByte = 0;

            if (_data != null)
            {
                retByte = _data.Contents[position];
            }
            else if (!(_dataString is null))
            {
                retByte = (byte)_dataString[position];
            }

            return retByte;
        }

        /// <summary>
        /// Gets the underlying ByteBuffer. Do not use the position and limit from
        /// the <see cref="ByteBuffer"/>. Use <see cref="Buffer.Position"/> and
        /// <see cref="Buffer.Length"/> from the <see cref="Buffer"/>.
        /// </summary>
        /// 
        /// <returns> <ol>
        /// <li>If this Buffer is backed by a <see cref="ByteBuffer"/>
        /// then the <see cref="ByteBuffer"/>'s reference will be returned.</li>
        /// <li>If this Buffer is backed by a String, then a new <see cref="ByteBuffer"/>
        /// will be created from a new <c>byte[]</c> from the backing String. Note that 
        /// this creates garbage.</li>
        /// </ol>
        /// </returns>
        /// 
        /// <seealso cref="ByteBuffer"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Data()
        {
            ByteBuffer buffer = null;

            if (_data != null)
            {
                buffer = _data;
            }
            else if (!string.ReferenceEquals(_dataString, null))
            {
                buffer = Copy();
            }

            return buffer;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal string DataString()
        {
            return _dataString;
        }

        // copy the references from srcBuffer to this buffer, without creating garbage.
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void CopyReferences(Buffer srcBuffer)
        {
            Buffer srcBuf = srcBuffer;

            _dataString = srcBuf.DataString();
            if (_dataString is null)
            {
                // backing buffer is not a String, safe to call data()
                _data = srcBuf.Data();
            }
            else
            {
                _data = null;
            }

            _position = srcBuf.Position;
            _length = srcBuf.GetLength();
        }

        /// <summary>
		/// Tests if this <see cref="Buffer"/> is equal to another <see cref="Buffer"/>. The two
		/// objects are equal if they have the same length and the two sequence of
		/// elements are equal.
		/// </summary>
        /// <remarks>
		/// <para>
		/// If one buffer is backed by a String and the other buffer is backed by a
		/// ByteBuffer, the String will be compared as 8-bit ASCII.
		/// 
		/// </para>
        /// </remarks>
		/// 
		/// <param name="thatBuffer"> The <c>Buffer</c> to compare with the current <c>Buffer</c>.
		/// </param>
		/// <returns> <c>true</c> if <c>Buffers</c> are equal, otherwise <c>false</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool Equals(Buffer thatBuffer)
        {
            if (thatBuffer != null && thatBuffer.Length == GetLength())
            {
                int thisLength = GetLength();
                // determine the backing buffers, in order to perform the compare.
                string thatString = thatBuffer.DataString();
                if (!(thatString is null))
                {
                    // thatBuffer is backed by a String
                    if (!(_dataString is null))
                    {
                        return _dataString.Equals(thatString); // both Strings
                    }

                    return CompareByteBufferToString(_data, _position, thisLength, thatString);
                }
                else
                {
                    // thatBuffer is backed by a ByteBuffer
                    if (!(_dataString is null))
                    {
                        return CompareByteBufferToString(thatBuffer.Data(), thatBuffer.Position, thatBuffer.GetLength(), _dataString);
                    }

                    // compare byte by byte, since RsslBuffer's pos and len may be separate from ByteBuffer.
                    int thisPosition = _position;
                    int thatPosition = thatBuffer.Position;
                    ByteBuffer thatByteBuffer = thatBuffer.Data();
                    for (int idx = 0; idx < thisLength; idx++)
                    {
                        if (_data.Contents[thisPosition + idx] != thatByteBuffer.Contents[thatPosition + idx])
                        {
                            return false;
                        }
                    }

                    return true;
                }
            }
            else
            {
                return false;
            }
        }

        /// <summary>
        /// Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.
        /// </summary>
        /// 
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.</param>
        /// 
        /// <returns><c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        ///   otherwise, <c>false</c>.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Buffer)
                return Equals((Buffer)other);

            return false;
        }

        /// <summary>
        /// Serves as a hash function for a particular type.
        /// </summary>
        /// <returns>A hash code for the current <c>Object</c>.
        /// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override int GetHashCode()
        {
            unchecked
            {
                int hashCode;

                if (!string.ReferenceEquals(_dataString, null))
                {
                    if (!_dataString.Equals(""))
                    {
                        hashCode = _dataString[0] + 31;
                        int multiplier = 1;
                        for (int i = 1; i < _dataString.Length; ++i)
                        {
                            multiplier *= 31;
                            hashCode += (_dataString[i] + 30) * multiplier;
                        }
                    }
                    else
                    {
                        hashCode = _data.Contents[_position] + 31;
                    }
                }
                else
                {
                    byte[] tmpByte = _data.Contents;
                    hashCode = tmpByte[_position] ^ 31;
                    int multiplier = 1;
                    for (int i = _position + 1; i < _position + Length; i++)
                    {
                        multiplier *= 31;
                        hashCode ^= (tmpByte[i] ^ 30) * multiplier;
                    }
                }

                return hashCode;
            }           
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static bool CompareByteBufferToString(ByteBuffer bb, int position, int length, string s)
        {
            if (bb is null || s is null)
            {
                return false;
            }

            if (length == s.Length)
            {

                for (int idx = 0; idx < length; idx++)
                {
                    if (bb.Contents[position + idx] != (unchecked((sbyte)(s[idx] & 0xFF))))
                    {
                        return false;
                    }
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        // returns false if the ByteBuffer is too small.
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private bool CopyStringToByteBuffer(string s, ByteBuffer bb)
        {
            int len = s.Length;
            if (len != 0)
            {
                if (len <= (bb.BufferLimit() - bb.BufferPosition()))
                {

                    for (int idx = 0; idx < len; idx++)
                    {
                        bb.WriteAt(idx, (unchecked((byte)(s[idx] & 0xFF))));
                    }
                    bb.WritePosition = len;

                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }

        // returns false if the ByteBuffer is too small.
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private bool CopyStringToByteArray(string s, byte[] ba, int offset)
        {
            int len = s.Length;
            if (len != 0)
            {

                if (len <= (ba.Length + offset))
                {
                    for (int idx = 0; idx < len; idx++)
                    {
                        ba[offset + idx] = unchecked((byte)(s[idx] & 0xFF));
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }

        /// <summary>
		/// Converts the underlying buffer into a formatted hexidecimal String. This
		/// method creates garbage.
		/// </summary>
		/// <returns> the formatted hexadecimal String representation.
		/// </returns>
		public string ToHexString()
        {
            if (_data == null && _dataString is null)
            {
                return null;
            }

            if (_dataString is null)
            {
                return ToHexString(_data, _position, _length);
            }
            else
            {
                // convert the _dataString into a ByteBuffer
                ByteBuffer buf = ByteBuffer.Wrap(Encoding.ASCII.GetBytes(_dataString));
                return ToHexString(buf, 0, (buf.BufferLimit() - buf.ReadPosition));
            }
        }

        /// <summary>
		/// Encodes a Buffer.
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter)
        {
            if (!IsBlank)
            {
                return Encoders.PrimitiveEncoder.EncodeBuffer((EncodeIterator)iter, this);
            }
            else
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }
        }

        /// <summary>
		/// Decodes a Buffer.
		/// </summary>
		/// <param name="iter"> The decoder iterator.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Decode(DecodeIterator iter)
        {
            return Decoders.DecodeBuffer(iter, this);
        }

        /// <summary>
		/// Returns the capacity of this buffer. If underlying data is backed by
		/// ByteBuffer, capacity is the difference between limit and initial position
		/// of the backing ByteBuffer. For String backed buffer, capacity is the string length.
		/// </summary>
		/// <value> Buffer capacity: number of bytes that this buffer can hold. </value>
		public int Capacity
        {
            get
            {
                if (_data != null)
                {
                    return _data.BufferLimit() - _position;
                }

                return _length;
            }
        }

        internal static string ToHexString(ByteBuffer buffer, int startPosition, int length)
        {
            const int charsPerLine = 16;
            StringBuilder asString = new StringBuilder();
            StringBuilder currentLine = new StringBuilder();
            StringBuilder all = new StringBuilder();

            bool processedFirst = false;
            int lineNo = 0;
            int currentChar = 0;

            // visit all the characters in the range
            for (int i = startPosition; i < (startPosition + length); i++)
            {
                if (!(currentChar < charsPerLine))
                {
                    // complete this line:
                    if (processedFirst)
                    {
                        all.Append(string.Format("\n{0:X4}: ", lineNo++));
                    }
                    else
                    {
                        all.Append(string.Format("{0:X4}: ", lineNo++));
                        processedFirst = true;
                    }

                    all.Append(currentLine.ToString()); // hex
                    all.Append("  "); // spacer
                    all.Append(asString.ToString());

                    // reset to prepare for the next line
                    currentLine.Length = 0;
                    asString.Length = 0;
                    currentChar = 0;
                }

                byte b = buffer._data[i];
                currentLine.Append(string.Format("{0:X2} ", b)); // convert the current byte to hex

                // prepare the byte to be printed as a string
                if (b > 31 && b < 127)
                {
                    asString.Append((char)b);
                }
                else
                {
                    asString.Append('.');
                }

                if (currentChar == 7)
                {
                    currentLine.Append(" "); // add an extra space after 8 chars
                }
                ++currentChar;
            }

            // process the last remaining line, if required
            if (currentLine.Length > 0)
            {
                if (processedFirst)
                {
                    all.Append("\n");
                }
                all.Append(string.Format("{0:X4}: ", lineNo++)); // append the current line number
                                                                 // fill in any unused chars
                int fill = currentChar;
                while (fill < charsPerLine)
                {
                    currentLine.Append("   ");

                    if (fill == 7)
                    {
                        currentLine.Append(" "); // add an extra space after 8 chars
                    }
                    ++fill;
                }

                all.Append(currentLine.ToString()); // hex
                all.Append("  "); // spacer
                all.Append(asString.ToString());
            }

            return all.ToString();
        }

        /// <summary>
        /// This is used internally to set a specific encoding name for the <see cref="ToString()"/>
        /// <remarks>
        /// This is used to override the default in order to encode non-ascii string.
        /// </remarks>
        /// </summary>
        /// <param name="encoding">The character encoding</param>
        internal Buffer(Encoding encoding)
        {
            m_Encoding = encoding;
        }

    }
}