/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using System.Linq;

using Refinitiv.Eta.Common.Interfaces;

namespace Refinitiv.Eta.Common
{
    /// <summary>
    /// This class provides the ability to read and write data to byte array using network byte order.
    /// </summary>
    public sealed class ByteBuffer : IDataBufferReader, IDataBufferWriter, IByteBuffer, IEquatable<ByteBuffer>, IDisposable
    {
#if DEBUG
        internal Guid BufferId { get; } = Guid.NewGuid();
#endif

        /// Represents the mode of ByteBuffer
        private enum Mode
        {
            Read,
            Write
        }

        private Mode _mode;

        private byte[] _data;

        private bool _isDataExternal;

        private bool disposed; // To detect redundant calls

        private int _limit; // -1 indicates the limit is not set

        /// <summary>
        /// Creates ByteBuffer with the specified capacity
        /// </summary>
        /// <param name="capacity">The capacity</param>
        public ByteBuffer(int capacity)
        {
            _data = new byte[capacity];
            _isDataExternal = false;
            _writePosition = 0;
            _readPosition = 0;
            _limit = -1;
            _mode = Mode.Write;
        }

        /// <summary>
        /// Creates <see cref="ByteBuffer"/> from the passed in byte array.
        /// </summary>
        /// <param name="data">The byte array</param>
        /// <param name="isEmpty">Is the byte array is empty</param>
        public ByteBuffer(byte[] data, bool isEmpty = false)
        {
            _data = data;
            _isDataExternal = true;

            if (!isEmpty)
                _writePosition = data?.Length ?? 0;
            else
                _writePosition = 0;

            _mode = Mode.Write;
            _limit = -1;
        }

        /// <summary>
        /// Gets the capacity of underlying byte array.
        /// </summary>
        public int Capacity { get => _data.Length; }

        /// <summary>
        /// Gets or sets the byte array.
        /// </summary>
        public byte[] Contents { get => _data; set => _data = value; }

        /// <summary>
        /// Gets the read limit of ByteBuffer
        /// </summary>
        int IDataBufferReader.Limit
        {
            get => WritePosition;
        }

        /// <summary>
        /// Gets the write limit of ByteBuffer
        /// </summary>
        int IDataBufferWriter.Limit
        {
            get => Capacity;
        }

        /// <summary>
        /// Gets the limit of ByteBuffer.
        /// </summary>
        /// <remarks>The return value depends on the state of ByteBuffer</remarks>
        public int Limit
        {
            get
            {
                if (_mode == Mode.Write)
                {
                    if (_limit == -1) // Checks whether the limit is set
                        return ((IDataBufferWriter)this).Limit;
                    else
                        return _limit;
                }
                else
                    return ((IDataBufferReader)this).Limit;
            }

            set
            {
                if (_mode == Mode.Write)
                {
                    if (value >= 0)
                        _limit = value;
                    else
                        throw new InvalidOperationException("The value for setting limit must not be less than zero.");
                }
                else
                    throw new InvalidOperationException("Not allowed to change the Limit on this interface.");
            }
        }

        /// <summary>
        /// Gets the current position of cursor.
        /// </summary>
        public int Position
        {
            get => (_mode == Mode.Read) ? ReadPosition : WritePosition;

            private set
            {
                if (_mode == Mode.Read)
                    ReadPosition = (int)value;
                else
                    WritePosition = (int)value;
            }
        }

        int _readPosition;

        /// <summary>
        /// Gets the read position
        /// </summary>
        public int ReadPosition
        {
            get => _readPosition;
            set
            {
                #if _SHOULD_BE_ENFORCED
                long writePosition = _writePosition;
                if (value > writePosition)
                    throw new EndOfStreamException($"Attempt to set ReadPosition ({value}) past WritePosition ({writePosition})");
                #endif
                _readPosition = value;
            }
        }

        int _writePosition;

        /// <summary>
        /// Gets the write position;
        /// </summary>
        public int WritePosition
        {
            get => _writePosition;
            set
            {
                _writePosition = value;
            }
        }

        /// <summary>
        ///  Gets the begining position
        /// </summary>
        public int Begin { get => 0; }

#region Contents Manipulation
        /// 
        /// Compact : Already read content, exit stage left!
        /// Clear   : Reset to initial state
        /// Reserve : Guarentee enouff room after WritePosition
        /// Truncate: Everything after the WritePosition, exit stage right!
        /// 
        /// <summary>
        /// Reset to initial state.
        /// </summary>
        public ByteBuffer Clear()
        {
            _mode = Mode.Write;
            ReadPosition = 0;
            WritePosition = 0;
            Contents.Initialize();
            return this;
        }

        /// <summary>
        /// Shift-left the already-read Contents.
        /// <code>
        /// Before: [ABCDEFG]
        ///            R    W
        ///  After: [CDEFG00]
        ///          R    W
        /// </code>
        /// </summary>
        public ByteBuffer Compact()
        {
            int dataSize = WritePosition - ReadPosition;

            WritePosition -= ReadPosition;
            Buffer.BlockCopy(Contents, ReadPosition, Contents, 0, dataSize);
            ReadPosition = Begin;

            _mode = Mode.Write;

            return this;
        }

        /// <summary>
        /// Reserve additional bytes in underlying data without advancing Position.
        /// </summary>
        /// <param name="count"></param>
        /// <returns></returns>
        public long Reserve(long count)
        {
            if (Position + count > Limit)
            {
                count -= (Limit - Position);
                var limit = Limit;
                byte[] data = _data;
                _data = new byte[Limit + count];
                Buffer.BlockCopy(data, 0, _data, 0, limit);

            }
            return Limit;
        }

        /// <summary>
        /// Shrink the Capacity to the WritePosition.
        /// </summary>
        /// <returns>ByteBuffer</returns>
        public ByteBuffer Truncate()
        {
            byte[] data = new byte[WritePosition];
            Buffer.BlockCopy(_data, 0, data, 0, WritePosition);
            _data = data;

            return this;
        }

#endregion

        /// <summary>
        /// Flip this buffer to the <see cref="Mode.Read"/> state for reading data at the beginning position.
        /// </summary>
        /// <returns>This object</returns>
        public ByteBuffer Flip()
        {
            _mode = Mode.Read;
            ReadPosition = Begin;
            return this;
        }

        /// <summary>
        /// Reset the read or write position depending upon read or write mode respectively
        /// </summary>
        /// <returns>This object</returns>
        public ByteBuffer Rewind()
        {
            if (_mode == Mode.Read)
            {
                _readPosition = 0;
            }
            else
            {
                _writePosition = 0;
            }

            return this;
        }

        /// <summary>
        /// 
        /// </summary>
        public int Remaining { get => Limit - Position; }

        /// <summary>
        /// 
        /// </summary>
        public bool HasRemaining
        {
            get => Remaining > 0;
        }

        /// <summary>
        /// Construct a new ByteBuffer wrapped around the passed in byte array.
        /// </summary>
        /// <param name="src">The source byte array</param>
        /// <returns>A newely created ByteBuffer</returns>
        public static ByteBuffer Wrap(byte[] src)
        {
            var byteBuffer = new ByteBuffer(src.Length);
            return byteBuffer.Put(src).Flip();
        }

#region IDataBufferReader
        /// --------------------------------------------------------------------------
        /// ReadXXX  :  Deserializes data from the Contents at the ReadPointer and
        ///             advances the ReadPointer.
        /// ReadXXXAt:  Deserializes data from the Contents at the given offset, but 
        ///             does not advance the ReadPointer.
        /// --------------------------------------------------------------------------
        public int ReadByte()
        {
            var value = ReadByteAt(ReadPosition);
            ReadPosition += sizeof(byte);
            return value;
        }

        /// <summary>
        /// Reads a byte from the specified position
        /// </summary>
        /// <param name="index">The position of byte array</param>
        /// <returns>The value</returns>
        public int ReadByteAt(int index)
        {
            if (index + sizeof(byte) > Limit)
                throw new EndOfStreamException();

            return Contents[index];
        }

        /// <summary>
        /// Copy data to another byte array
        /// </summary>
        /// <param name="destination">The desination byte array</param>
        /// <param name="destinationOffset">The desination offset</param>
        /// <param name="length"></param>
        /// <returns>This object</returns>
        public ByteBuffer ReadBytesInto(byte[] destination, int destinationOffset, int length)
        {
            if (destination.Length < destinationOffset + length)
                throw new ArgumentOutOfRangeException($"{nameof(destinationOffset)} + {nameof(length)} > {nameof(destination)}.Length ({destination.Length})");
            if (destinationOffset < 0)
                throw new ArgumentOutOfRangeException($"{nameof(destinationOffset)}");
            if (length < 0)
                throw new ArgumentOutOfRangeException($"{nameof(length)}");

            if (ReadPosition < Limit)
            {
                length = (int)(((Limit - ReadPosition) > length)
                        ? length
                        : (Limit - ReadPosition));
                Buffer.BlockCopy(Contents, ReadPosition, destination, destinationOffset, length);
                ReadPosition += length;
            }
            return this;
        }

        /// <summary>
        /// Read current postion as <see cref="double"/>
        /// </summary>
        /// <returns><see cref="double"/></returns>
        public double ReadDouble()
        {
            var value = ReadDoubleAt(ReadPosition);
            ReadPosition += sizeof(double);
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="double"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="double"/></returns>
        public double ReadDoubleAt(int index)
        {
            if (index + sizeof(double) > Limit)
                throw new EndOfStreamException();

            byte[] byteArray = new byte[sizeof(double)];
            Buffer.BlockCopy(Contents, (int)index, byteArray, 0, sizeof(double));
            Array.Reverse(byteArray);
            return BitConverter.ToDouble(byteArray, 0);
        }

        /// <summary>
        /// Read current postion as <see cref="float"/>
        /// </summary>
        /// <returns><see cref="float"/></returns>
        public float ReadFloat()
        {
            var value = ReadFloatAt(ReadPosition);
            ReadPosition += sizeof(float);
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="float"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="float"/></returns>
        public float ReadFloatAt(int index)
        {
            if (index + sizeof(float) > Limit)
                throw new EndOfStreamException();

            byte[] byteArray = new byte[sizeof(float)];
            Buffer.BlockCopy(Contents, (int)index, byteArray, 0, sizeof(float));
            Array.Reverse(byteArray);
            return BitConverter.ToSingle(byteArray, 0);
        }

        /// <summary>
        /// Read current postion as <see cref="Int32"/>
        /// </summary>
        /// <returns><see cref="Int32"/></returns>
        public Int32 ReadInt()
        {
            var value = ReadIntAt(ReadPosition);
            ReadPosition += sizeof(Int32);
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="Int32"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="Int32"/></returns>
        public Int32 ReadIntAt(int index)
        {
            if (index + sizeof(Int32) > Limit)
                throw new EndOfStreamException();

            return System.Net.IPAddress.NetworkToHostOrder(BitConverter.ToInt32(Contents, index));
        }

        /// <summary>
        /// Read current postion as <see cref="UInt32"/>
        /// </summary>
        /// <returns><see cref="UInt32"/></returns>
        public UInt32 ReadUInt()
        {
            var value = ReadUIntAt(ReadPosition);
            ReadPosition += sizeof(UInt32);
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="UInt32"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="UInt32"/></returns>
        public UInt32 ReadUIntAt(int index)
        {
            if (index + sizeof(UInt32) > Limit)
                throw new EndOfStreamException();

            long value = System.Net.IPAddress.NetworkToHostOrder(BitConverter.ToInt32(Contents, index));

            if(value < 0)
            {
                // convert to unsigned.
                value &= 0xFFFFFFFF;
            }

            return (UInt32)value;
        }

        /// <summary>
        /// Read current postion as <see cref="Int64"/>
        /// </summary>
        /// <returns><see cref="Int64"/></returns>
        public Int64 ReadLong()
        {
            var value = ReadLongAt(ReadPosition);
            ReadPosition += sizeof(Int64);
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="Int64"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="Int64"/></returns>
        public long ReadLongAt(int index)
        {
            if (index + sizeof(long) > Limit)
                throw new EndOfStreamException();

            return System.Net.IPAddress.NetworkToHostOrder(BitConverter.ToInt64(Contents, index));
        }

        /// <summary>
        /// Read current postion as <see cref="Int16"/>
        /// </summary>
        /// <returns><see cref="Int16"/></returns>
        public short ReadShort()
        {
            var value = ReadShortAt(ReadPosition);
            ReadPosition += sizeof(short);
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="Int16"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="Int16"/></returns>
        public Int16 ReadShortAt(int index)
        {
            if (index + sizeof(Int16) > Limit)
                throw new EndOfStreamException();

            return System.Net.IPAddress.NetworkToHostOrder(BitConverter.ToInt16(Contents, index));
        }

        /// <summary>
        /// Read current postion as <see cref="UInt16"/>
        /// </summary>
        /// <returns><see cref="UInt16"/></returns>
        public ushort ReadUShort()
        {
            var value = ReadUShortAt(ReadPosition);
            ReadPosition += sizeof(ushort);
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="UInt16"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="UInt16"/></returns>
        public ushort ReadUShortAt(int index)
        {
            if (index + sizeof(UInt16) > Limit)
                throw new EndOfStreamException();

            int val = System.Net.IPAddress.NetworkToHostOrder(BitConverter.ToInt16(Contents, index));

            if(val < 0)
            {
                // convert to unsigned.
                val &= 0xFFFF;
            }

            return (ushort)val;
        }

        #endregion

        #region IDataBufferWriter
        // ---------------------------------------------------------------------
        // Put       : Slam bytes from given byte[] into Contents.
        // WriteXXX  : Serializes data into the Contents at the WritePointer, and
        //             advances the WritePointer
        // WriteXXXAt: Serializes data int the Contents at the given offset, but
        //             does not advance the WritePointer.
        // ---------------------------------------------------------------------

        #region Stream Access

        /// <summary>
        /// Copies the content from the byte array
        /// </summary>
        /// <param name="source">The source byte array</param>
        /// <param name="offset">The offset of the source</param>
        /// <param name="length">The length to copy from</param>
        /// <returns>This object</returns>
        public ByteBuffer Put(byte[] source, int offset, int length)
        {
            Reserve(length);
            Buffer.BlockCopy(source, offset, Contents, WritePosition, length);
            WritePosition += length;
            return this;
        }

        /// <summary>
        /// Copy content from a byte array.
        /// </summary>
        /// <param name="source">The byte array to copy content from</param>
        /// <returns>This object</returns>
        public ByteBuffer Put(byte[] source)
        {
            return Put(source, 0, source.Length);
        }

        /// <summary>
        /// Copy content from a <see cref="ByteBuffer"/>
        /// </summary>
        /// <param name="sourceBuffer">The <see cref="ByteBuffer"/> to copy content from</param>
        /// <returns>This object</returns>
        public ByteBuffer Put(ByteBuffer sourceBuffer)
        {
            Put(sourceBuffer.Contents, sourceBuffer.ReadPosition, sourceBuffer.WritePosition - sourceBuffer.ReadPosition);
            return this;
        }

#endregion

#region StreamWriter - Serialization

        /// <summary>
        /// Write a <see cref="byte"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="byte"/> to write</param>
        /// <returns>This object</returns>
        public ByteBuffer Write(byte value)
        {
            WriteAt(WritePosition, value);
            WritePosition++;
            return this;
        }

        /// <summary>
        /// Write a <see cref="byte"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="byte"/> value</param>
        /// <returns>This object</returns>
        public ByteBuffer WriteAt(int index, byte value)
        {
            if (index + sizeof(byte) > Limit)
                throw new EndOfStreamException();
            Contents[index] = value;
            return this;
        }

        /// <summary>
        /// Write a <see cref="double"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="double"/> to write</param>
        /// <returns>This object</returns>
        public ByteBuffer Write(double value)
        {
            Reserve(sizeof(double));
            WriteAt(WritePosition, value);
            WritePosition += sizeof(double);
            return this;
        }

        /// <summary>
        /// Write a <see cref="double"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="double"/> value</param>
        /// <returns>This object</returns>
        public ByteBuffer WriteAt(int index, double value)
        {
            if (index + sizeof(double) > Limit)
                throw new EndOfStreamException();

            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, Contents, index, sizeof(double));
            Array.Reverse(Contents, index, sizeof(double));
            return this;
        }

        /// <summary>
        /// Write a <see cref="float"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="float"/> to write</param>
        /// <returns>This object</returns>
        public ByteBuffer Write(float value)
        {
            Reserve(sizeof(float));
            WriteAt(WritePosition, value);
            WritePosition += sizeof(float);
            return this;
        }

        /// <summary>
        /// Write a <see cref="float"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="float"/> value</param>
        /// <returns>This object</returns>
        public ByteBuffer WriteAt(int index, float value)
        {
            if (index + sizeof(float) > Limit)
                throw new EndOfStreamException();

            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, Contents, index, sizeof(float));
            Array.Reverse(Contents, index, sizeof(float));
            return this;
        }

        /// <summary>
        /// Write a <see cref="Int32"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="Int32"/> to write</param>
        /// <returns>This object</returns>
        public ByteBuffer Write(Int32 value)
        {
            Reserve(sizeof(int));
            WriteAt(WritePosition, value);
            WritePosition += sizeof(int);
            return this;
        }

        private readonly byte[] fourBytes = new byte[4];

        /// <summary>
        /// Write a <see cref="Int32"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="Int32"/> value</param>
        /// <returns>This object</returns>
        public unsafe ByteBuffer WriteAt(int index, Int32 value)
        {
            int convertedValue = System.Net.IPAddress.HostToNetworkOrder(value);
            fixed (byte* b = fourBytes)
                *((int*)b) = convertedValue;

            for (int i = 0; i < 4; i++)
                Contents[i + index] = fourBytes[i];

            return this;
        }

        /// <summary>
        /// Write a <see cref="Int16"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="Int16"/> to write</param>
        /// <returns>This object</returns>
        public ByteBuffer Write(Int16 value)
        {
            Reserve(sizeof(short));
            WriteAt(WritePosition, value);
            WritePosition += sizeof(short);
            return this;
        }

        private readonly byte[] twoBytes = new byte[2];

        /// <summary>
        /// Write a <see cref="Int16"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="Int16"/> value</param>
        /// <returns>This object</returns>
        public unsafe ByteBuffer WriteAt(int index, short value)
        {
            short convertedValue = System.Net.IPAddress.HostToNetworkOrder(value);
            fixed (byte* b = twoBytes)
                *((short*)b) = convertedValue;

            for (int i = 0; i < 2; i++)
                Contents[i + index] = twoBytes[i];

            return this;
        }

        /// <summary>
        /// Write a <see cref="Int64"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="Int64"/> to write</param>
        /// <returns>This object</returns>
        public ByteBuffer Write(long value)
        {
            Reserve(sizeof(long));
            WriteAt(WritePosition, value);
            WritePosition += sizeof(long);
            return this;
        }

        private readonly byte[] eightBytes = new byte[8];

        /// <summary>
        /// Write a <see cref="Int64"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="Int64"/> value</param>
        /// <returns>This object</returns>
        public unsafe ByteBuffer WriteAt(int index, long value)
        {
            long convertedValue = System.Net.IPAddress.HostToNetworkOrder(value);
            fixed (byte* b = eightBytes)
            *((long*)b) = convertedValue;
            for (int i = 0; i < 8; i++)
                Contents[i + index] = eightBytes[i];

            return this;
        }

        #endregion

        #endregion

        #region IEquatable

        /// <summary>
        /// Indicates whether the current object is equal to another <see cref="ByteBuffer"/>
        /// </summary>
        /// <param name="other">The <see cref="ByteBuffer"/> to compare</param>
        /// <returns><c>true</c> if the current object is equal to the other parameter; otherwise, <c>false</c>.</returns>
        public bool Equals(ByteBuffer other)
        {
            if (this == other)
                return true;
          
            if (other != null && WritePosition == other.WritePosition)
            {
                return Contents.Take(WritePosition)
                                .SequenceEqual(other.Contents.Take(WritePosition));
            }

            return false;
        }

        #endregion

        void IDisposable.Dispose()
        {
            if (disposed)
                return;

            disposed = true;
            if (!_isDataExternal)
                _data = null;
        }

        /// <summary>
        /// Gets string representation of this object
        /// </summary>
        /// <returns><see cref="string"/></returns>
        public override string ToString()
        {
#if DEBUG
            var byteStrings = (Contents.Take((Contents.Length < 32) ? Contents.Length : 32).Select(b => $"{b:X2}"));
            string payload = String.Join(", ", byteStrings);
            return $"ReadPosition: {ReadPosition}, WritePosition: {WritePosition}, Limit: {Limit}, Contents[{Contents?.Length}]\n{{ {payload} }}";
#endif
#if RELEASE
            return $"ReadPosition: {ReadPosition}, WritePosition: {WritePosition}, Limit: {Limit}, Contents[{Contents?.Length}]";
#endif
        }

        internal string ToString(bool full)
        {
            var byteStrings = Contents.Select(b => $"{b:X2}");
            string payload = String.Join(", ", byteStrings);
            return $"ReadPosition: {ReadPosition}, WritePosition: {WritePosition}, Limit: {Limit}, Contents[{Contents?.Length}]\n{{ {payload} }}";
        }
    }
}
