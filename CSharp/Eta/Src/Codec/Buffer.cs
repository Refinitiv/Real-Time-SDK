/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System.Text;
using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Codec
{
    /// <summary>
	/// ETA Buffer represents some type of user-provided content along with the
	/// content's length.
	/// 
	/// <para>
	/// Used by data encoder/decoder and message packages. The <code>Buffer</code>
	/// has a position and length. When accessing the backing data, use
	/// <seealso cref="Buffer.Position"/> for the position and <seealso cref="Buffer.Length"/> for
	/// the length, not the position and limit of the <seealso cref="ByteBuffer"/> returned
	/// from <seealso cref="Buffer.Data()"/>.Blank buffers are conveyed as a <seealso cref="Buffer.Length"/>
	/// of 0.
	/// 
	/// </para>
	/// <para>
	/// <seealso cref="Buffer"/> can:
	/// <ul>
	/// <li>
	/// Represent various buffer and string types, such as ASCII, RMTES, or UTF8
	/// strings.</li>
	/// <li>
	/// Contain or reference encoded data on both container and message header
	/// structures.</li>
	/// </ul>
	/// </para>
	/// </summary>
	sealed public class Buffer
    {
        /* The length of the buffer. */
        private int _length;
        /* The position of the data. */
        private int _position;
        /* The actual data represented as a ByteBuffer. */
        internal ByteBuffer _data;
        /* The actual data represented as a String. */
        private string _dataString;

        internal bool _isBlank;

        /// <summary>
        /// Creates <seealso cref="Buffer"/> with no data.
        /// must be called later for buffer to be useful.
        /// </summary>
        /// <returns> Buffer object
        /// </returns>
        /// <seealso cref="Buffer"/>
        public Buffer()
        {
        }

        /// <summary>
		/// Sets the Buffer data to the ByteBuffer. This buffer's position and length
		/// will be taken from the specified ByteBuffer.
		/// </summary>
		/// <param name="data"> the <seealso cref="ByteBuffer"/> data to set.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success, or
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if data is null. </returns>
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

        internal void Data_internal(ByteBuffer data)
        {
            _data = data;
            _dataString = null;
            _position = data.Position;
            _length = data.Limit - _position;
            _isBlank = false;
        }

        /// <summary>
        /// Sets the Buffer data to the ByteBuffer. This buffer's position and length
        /// will be set to the specified position and length.
        /// </summary>
        /// <param name="data"> the <seealso cref="ByteBuffer"/> to set. </param>
        /// <param name="position"> the data's starting position. </param>
        /// <param name="length"> the data's length.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
        ///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if data is null, or if
        ///         position or length is outside of the data's capacity. </returns>
        public CodecReturnCode Data(ByteBuffer data, int position, int length)
        {
            if (data != null)
            {
                if (position >= 0 && position <= data.Limit && length >= 0 && (position + length) <= data.Limit)
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
        /// <param name="destBuffer"> A <seealso cref="ByteBuffer"/> large enough
        ///            to hold the contents of this <seealso cref="Buffer"/>.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
        ///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if the destBuffer is
        ///         null, or <seealso cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the
        ///         destBuffer is too small.
        /// </returns>
        /// <seealso cref="Buffer.Length"/>
        public CodecReturnCode Copy(ByteBuffer destBuffer)
        {
            if (destBuffer is null)
                return CodecReturnCode.INVALID_ARGUMENT;

            if (Length == 0)
                return CodecReturnCode.SUCCESS;

            if (Length > destBuffer.Remaining)
                return CodecReturnCode.BUFFER_TOO_SMALL;

            int length = Length;

            if (_dataString is null)
            {
                // ByteBuffer to ByteBuffer.
                destBuffer.Put(_data.Contents, _position, length);
            }
            else
            {
                // no need to check return code since we did range checking already.
                CopyStringToByteBuffer(_dataString, destBuffer);
            }

            return CodecReturnCode.SUCCESS;
        }

        internal CodecReturnCode Copy(byte[] destBuffer)
        {
            return Copy(destBuffer, 0);
        }

        internal CodecReturnCode Copy(byte[] destBuffer, int destOffset)
        {
            int length = Length;
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
		/// <param name="destBuffer"> A <seealso cref="Buffer"/> backed by a
		///            <seealso cref="ByteBuffer"/> large enough to hold
		///            the contents of this <seealso cref="Buffer"/>.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if the destBuffer is
		///         null, the backing buffer is null or the backing buffer is a
		///         String, or <seealso cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the
		///         destBuffer is too small.
		/// </returns>
		/// <seealso cref="Buffer.Length"/>
		/// <seealso cref="Buffer.Position"/>
		public CodecReturnCode Copy(Buffer destBuffer)
        {
            int length = Length;
            if (length != 0)
            {
                if (destBuffer != null)
                {
                    if ((destBuffer).DataString() is null)
                    {
                        ByteBuffer destByteBuffer = destBuffer.Data();
                        if (destByteBuffer != null)
                        {
                            if (length <= destBuffer.Length)
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

        internal CodecReturnCode CopyWithOrWithoutByteBuffer(Buffer buffer)
        {
            if (_data != null)
            {
                if (buffer._data == null || Length > buffer.Length)
                {
                    ByteBuffer newByteBuffer = new ByteBuffer(Length);
                    buffer.Data(newByteBuffer);
                }
            } else if (_dataString != null)
            {
                buffer.Data_internal(_dataString);
                return CodecReturnCode.SUCCESS;
            }
            return Copy(buffer);
        }

        private ByteBuffer Copy()
        {
            return ByteBuffer.Wrap(DataBytes());
        }

        /// <summary>
		/// The backing ByteBuffer is initially set along with initial position and length. 
		/// This method returns the initial length if there was no operation on the backing 
		/// ByteBuffer that would change the position (such as get or put).
		/// If the backing ByteBuffer position has been changed by reading or writing to 
		/// the buffer, this method returns the change in position, i.e. difference between 
		/// current position and initial position.
		/// </summary>
		/// <returns> the length </returns>
		public int Length
        {
            get
            {
                int len = _length;

                if (_data != null && _data.Position > _position)
                {
                    if (_data.Position - _position < len)
                    {
                        len = (int)_data.Position - _position;
                    }
                }

                return len;
            }
        }

        /// <summary>
		/// Returns the initial position of the buffer.
		/// </summary>
		/// <returns> the position </returns>
		public int Position { get => _position; }

        /// <summary>
        /// Converts the underlying buffer into a String. This should only be called
        /// when the Buffer is known to contain ASCII data.This method creates
        /// garbage unless the underlying buffer is a String.
        /// </summary>
        /// <returns>The String representation</returns>
        public override string ToString()
        {
            string retStr = null;

            if (_data != null)
            {
                retStr = System.Text.Encoding.ASCII.GetString(DataBytes(), 0, _length);
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
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success, or
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if data is null. </returns>
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

        internal void Data_internal(string str)
        {
            _dataString = str;
            _data = null;
            _position = 0;
            _length = str.Length;
            _isBlank = false;
        }

        /* Appends a byte to the buffer. */
        internal void AppendByte(byte b)
        {
            _data.Write(b);
        }

        /// <summary>
		/// Clears the buffer by setting its underlying ByteBuffer to null and its
		/// length and position to 0. If the underlying ByteBuffer is not referenced 
		/// in any other place, it will be a subject to GC.
		/// </summary>
		public void Clear()
        {
            _data = null;
            _dataString = null;
            _length = 0;
            _position = 0;
        }

        internal void Blank()
        {
            Clear();
            _isBlank = true;
        }

        /// <summary>
        /// Returns true if the Buffer has been decoded as blank.
        /// </summary>
        public bool IsBlank { get => _isBlank; }

        /// <summary>
        /// Return a copy of the Data/DataString.
        /// </summary>
        /// <returns></returns>
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
            } else
            {
                bufferBytes = new byte[_length];
            }
            return bufferBytes;
        }

        /* Gets a data byte from a position in the buffer. */
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
		/// the <seealso cref="ByteBuffer"/>. Use <seealso cref="Buffer.Position"/> and
		/// <seealso cref="Buffer.Length"/> from the <seealso cref="Buffer"/>.
		/// </summary>
		/// <returns> <ol>
		/// <li>If this Buffer is backed by a <seealso cref="ByteBuffer"/>
		/// then the <seealso cref="ByteBuffer"/>'s reference will be returned.</li>
		/// <li>If this Buffer is backed by a String, then a new <seealso cref="ByteBuffer"/>
        /// will be created from a new byte[] from the backing String. Note that 
        /// this creates garbage.</li>
		/// </ol> </returns>
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

        internal string DataString()
        {
            return _dataString;
        }

        // copy the references from srcBuffer to this buffer, without creating garbage.
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
            _length = srcBuf.Length;
        }

        /// <summary>
		/// Tests if this <seealso cref="Buffer"/> is equal to another <seealso cref="Buffer"/>. The two
		/// objects are equals if they have the same length and the two sequence of
		/// elements are equal.
		/// <para>
		/// If one buffer is backed by a String and the other buffer is backed by a
		/// ByteBuffer, the String will be compared as 8-bit ASCII.
		/// 
		/// </para>
		/// </summary>
		/// <param name="buffer"> A Buffer.
		/// </param>
		/// <returns> true if equals, otherwise false. </returns>
		public bool Equals(Buffer buffer)
        {
            if (buffer != null && buffer.Length == _length)
            {
                // determine the backing buffers, in order to perform the compare.
                Buffer thatBuffer = (Buffer)buffer;
                string thatString = thatBuffer.DataString();
                if (!(thatString is null))
                {
                    // thatBuffer is backed by a String
                    if (!(_dataString is null))
                    {
                        return _dataString.Equals(thatString); // both Strings
                    }

                    return CompareByteBufferToString(_data, _position, _length, thatString);
                }
                else
                {
                    // thatBuffer is backed by a ByteBuffer
                    if (!(_dataString is null))
                    {
                        return CompareByteBufferToString(thatBuffer.Data(), thatBuffer.Position, thatBuffer.Length, _dataString);
                    }

                    // compare byte by byte, since RsslBuffer's pos and len may be separate from ByteBuffer.
                    int thisPosition = _position;
                    int thatPosition = thatBuffer.Position;
                    ByteBuffer thatByteBuffer = thatBuffer.Data();
                    for (int idx = 0; idx < _length; idx++)
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

        /// <summary>Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.</summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.</param>
        /// <returns><c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        /// otherwise, <c>false</c>.</returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Buffer)
                return Equals((Buffer)other);

            return false;
        }

        /// <summary>Serves as a hash function for a particular type.</summary>
        /// <returns>A hash code for the current <c>Object</c>.</returns>
		public override int GetHashCode()
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
                hashCode = _data.Contents[_position] + 31;
                int multiplier = 1;
                for (int i = _position + 1; i < _position + _length; ++i)
                {
                    multiplier *= 31;
                    hashCode += (_data.Contents[i] + 30) * multiplier;
                }
            }

            return hashCode;
        }

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
        private bool CopyStringToByteBuffer(string s, ByteBuffer bb)
        {
            int len = s.Length;
            if (len != 0)
            {
                if (len <= (bb.Limit - bb.Position))
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
		/// <returns> the formatted hexadecimal String representation. </returns>
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
                return ToHexString(buf, 0, (buf.Limit - buf.ReadPosition));
            }
        }

        /// <summary>
		/// Encodes a Buffer.
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
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
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
        {
            return Decoders.DecodeBuffer(iter, this);
        }

        /// <summary>
		/// Returns the capacity of this buffer. If underlying data is backed by
		/// ByteBuffer, capacity is the difference between limit and initial position
		/// of the backing ByteBuffer. For String backed buffer, capacity is the string length.
		/// </summary>
		/// <returns> the capacity. Number of bytes that this buffer can hold. </returns>
		public int Capacity
        {
            get
            {
                if (_data != null)
                {
                    return (int)_data.Limit - _position;
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

                byte b = buffer.Contents[i];
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

    }

}