/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace LSEG.Eta.Common
{
    /// <summary>
    /// This class provides the ability to read and write data to byte array using network byte order.
    /// NOTE: the class pins the underlying byte array using <see cref="GCHandle"/> instance
    /// and creates a pointer to the byte array for better performance. 
    /// In case deterministic behavior is needed, call <see cref="Dispose()"/> method, 
    /// otherwise the handle will be freed in the finalizer of the class at undetermined moment in time.
    /// </summary>
    public unsafe sealed class ByteBuffer : IDataBufferReader, IDataBufferWriter, IByteBuffer, IEquatable<ByteBuffer>, IDisposable
    {
#if DEBUG
        internal Guid BufferId { get; } = Guid.NewGuid();
#endif

        /// <summary>
        /// Represents the mode of ByteBuffer
        /// </summary>
        public enum BufferMode
        {
            /// <summary>
            /// Indicates that the ByteBuffer is in read mode.
            /// </summary>
            Read,
            /// <summary>
            /// Indicates that the ByteBuffer is in write mode.
            /// </summary>
            Write
        }

        /// <summary>
        /// Gets the current mode of this object.
        /// </summary>
        public BufferMode Mode { get; private set; }

        private bool _isDataExternal;

        private bool disposed; // To detect redundant calls

        private int _limit; // -1 indicates the limit is not set

        internal GCHandle _handle;

        internal byte* _pointer;

        internal byte[] _data;

        /// <summary>
        /// Creates ByteBuffer with the specified capacity
        /// </summary>
        /// <param name="capacity">The capacity</param>
        public ByteBuffer(int capacity)
        {
            _data = GC.AllocateArray<byte>(capacity, true);
            _handle = GCHandle.Alloc(_data, GCHandleType.Pinned);
            _pointer = (byte*)_handle.AddrOfPinnedObject();
            _isDataExternal = false;
            WritePosition = 0;
            _readPosition = 0;
            _limit = -1;
            Mode = BufferMode.Write;
        }

        /// <summary>
        /// Creates <see cref="ByteBuffer"/> from the passed in byte array.
        /// </summary>
        /// <param name="data">The byte array</param>
        /// <param name="isEmpty">Is the byte array is empty</param>
        public ByteBuffer(byte[] data, bool isEmpty = false)
        {
            _data = data;
            if (data != null)
            {
                _handle = GCHandle.Alloc(_data, GCHandleType.Pinned);
                _pointer = (byte*)_handle.AddrOfPinnedObject();
            }
            _isDataExternal = true;

            if (!isEmpty)
                WritePosition = data?.Length ?? 0;
            else
                WritePosition = 0;

            Mode = BufferMode.Write;
            _limit = -1;
        }

        /// <summary>
        /// Gets the capacity of underlying byte array.
        /// </summary>
        public int Capacity { get => _data.Length; }

        /// <summary>
        /// Gets or sets the byte array.
        /// </summary>
        public byte[] Contents 
        { 
            get => _data; 
        }

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
                if (Mode == BufferMode.Write)
                {
                    if (_limit == -1) // Checks whether the limit is set
                        return Capacity;
                    else
                        return _limit;
                }
                else
                    return WritePosition;
            }

            set
            {
                if (Mode == BufferMode.Write)
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
        /// Gets the current ByteBuffer limit
        /// </summary>
        /// <returns>The limit of this buffer according to <see cref="BufferMode"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int BufferLimit()
        {
            if (Mode == BufferMode.Write)
            {
                if (_limit == -1) // Checks whether the limit is set
                    return Capacity;
                else
                    return _limit;
            }
            else
                return WritePosition;
        }

        /// <summary>
        /// Gets the current position of cursor.
        /// </summary>
        public int Position
        {
            get => (Mode == BufferMode.Read) ? ReadPosition : WritePosition;

            private set
            {
                if (Mode == BufferMode.Read)
                    ReadPosition = value;
                else
                    WritePosition = value;
            }
        }

        /// <summary>
        /// Gets the current ByteBuffer position
        /// </summary>
        /// <returns>ByteBuffer position</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int BufferPosition()
        {
            return (Mode == BufferMode.Read) ? ReadPosition : WritePosition;
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

        /// <summary>
        /// Gets the write position;
        /// </summary>
        public int WritePosition;

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
        /// <returns>This object instance.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Clear()
        {
            Mode = BufferMode.Write;
            ReadPosition = 0;
            WritePosition = 0;
            _data.Initialize();
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
        /// <returns>This object instance.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Compact()
        {
            int dataSize = WritePosition - ReadPosition;

            WritePosition -= ReadPosition;
            Buffer.BlockCopy(_data, ReadPosition, _data, 0, dataSize);
            ReadPosition = Begin;

            Mode = BufferMode.Write;

            return this;
        }

        /// <summary>
        /// Reserve additional bytes in underlying data without advancing Position.
        /// </summary>
        /// <param name="count">The number of bytes to reserve</param>
        /// <returns>The limit of this buffer</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long Reserve(long count)
        {
            var limit = BufferLimit();
            if (BufferPosition() + count > limit)
            {           
                count -= limit - Position;      
                byte[] data = _data;
                if (_handle.IsAllocated) _handle.Free();
                _data = new byte[limit + count];
                _handle = GCHandle.Alloc(_data, GCHandleType.Pinned);
                _pointer = (byte*)_handle.AddrOfPinnedObject();
                Buffer.BlockCopy(data, 0, _data, 0, limit);
            }
            return BufferLimit();
        }

        /// <summary>
        /// Shrink the Capacity to the WritePosition.
        /// </summary>
        /// <returns>ByteBuffer</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Truncate()
        {
            byte[] data = new byte[WritePosition];
            Buffer.BlockCopy(_data, 0, data, 0, WritePosition);
            if (_handle.IsAllocated) _handle.Free();
            _data = data;
            _handle = GCHandle.Alloc(_data, GCHandleType.Pinned);
            _pointer = (byte*)_handle.AddrOfPinnedObject();

            return this;
        }

        #endregion

        /// <summary>
        /// Flip this buffer to the <see cref="BufferMode.Read"/> state for reading data at the beginning position.
        /// </summary>
        /// <returns>This object instance</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Flip()
        {
            Mode = BufferMode.Read;
            ReadPosition = Begin;
            return this;
        }

        /// <summary>
        /// Reset the read or write position depending upon read or write mode respectively
        /// </summary>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Rewind()
        {
            if (Mode == BufferMode.Read)
            {
                _readPosition = 0;
            }
            else
            {
                WritePosition = 0;
            }

            return this;
        }

        /// <summary>
        /// Gets the number of remaining bytes.
        /// </summary>
        public int Remaining { get => BufferLimit() - BufferPosition(); }

        /// <summary>
        /// Checks whether this buffer has remaining number of bytes.
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
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static ByteBuffer Wrap(byte[] src)
        {
            var byteBuffer = new ByteBuffer(src.Length);
            return byteBuffer.Put(src).Flip();
        }

        #region IDataBufferReader
        /// <summary>
        /// Read current postion as byte.
        /// </summary>
        /// <returns><see cref="byte"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public byte ReadByte()
        {
#if DEBUG
            if (Mode == BufferMode.Read && ReadPosition + 1 > WritePosition || ReadPosition + 1 > BufferLimit())
                throw new EndOfStreamException();
#endif
            return _pointer[ReadPosition++];
        }

        /// <summary>
        /// Reads a byte from the specified position
        /// </summary>
        /// <param name="index">The position of byte array</param>
        /// <returns>The value</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int ReadByteAt(int index)
        {
#if DEBUG
            if (Mode == BufferMode.Read && index + 1 > WritePosition || index + 1 > BufferLimit())
                throw new EndOfStreamException();
#endif
            return _pointer[index];
        }

        /// <summary>
        /// Copy data to another byte array
        /// </summary>
        /// <param name="destination">The desination byte array</param>
        /// <param name="destinationOffset">The desination offset</param>
        /// <param name="length"></param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer ReadBytesInto(byte[] destination, int destinationOffset, int length)
        {
            if (destination.Length < destinationOffset + length)
                throw new ArgumentOutOfRangeException($"{nameof(destinationOffset)} + {nameof(length)} > {nameof(destination)}.Length ({destination.Length})");
            if (destinationOffset < 0)
                throw new ArgumentOutOfRangeException($"{nameof(destinationOffset)}");
            if (length < 0)
                throw new ArgumentOutOfRangeException($"{nameof(length)}");

            if (ReadPosition < BufferLimit())
            {
                length = BufferLimit() - ReadPosition > length
                        ? length
                        : BufferLimit() - ReadPosition;
                Buffer.BlockCopy(_data, ReadPosition, destination, destinationOffset, length);
                ReadPosition += length;
            }
            return this;
        }

        /// <summary>
        /// Read current postion as <see cref="double"/>
        /// </summary>
        /// <returns><see cref="double"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public double ReadDouble()
        {
            var value = ReadDoubleAt(ReadPosition);
            ReadPosition += 8;
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="double"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="double"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public double ReadDoubleAt(int index)
        {
#if DEBUG
            if (index + 8 > BufferLimit())
                throw new EndOfStreamException();
#endif

            byte[] byteArray = new byte[8];
            Buffer.BlockCopy(_data, index, byteArray, 0, 8);
#if !BIGENDIAN
            Array.Reverse(byteArray);
#endif
            return BitConverter.ToDouble(byteArray, 0);
        }

        /// <summary>
        /// Read current postion as <see cref="float"/>
        /// </summary>
        /// <returns><see cref="float"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public float ReadFloat()
        {
            var value = ReadFloatAt(ReadPosition);
            ReadPosition += 4;
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="float"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="float"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public float ReadFloatAt(int index)
        {
#if DEBUG
            if (index + 4 > BufferLimit())
                throw new EndOfStreamException();
#endif

            byte[] byteArray = new byte[4];
            Buffer.BlockCopy(_data, index, byteArray, 0, 4);
#if !BIGENDIAN
            Array.Reverse(byteArray);
#endif
            return BitConverter.ToSingle(byteArray, 0);
        }

        /// <summary>
        /// Read current postion as <see cref="int"/>
        /// </summary>
        /// <returns><see cref="int"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int ReadInt()
        {
#if DEBUG
            if (Mode == BufferMode.Read && ReadPosition + 4 > WritePosition || ReadPosition + 4 > BufferLimit())
                throw new EndOfStreamException();
#endif

            int value = 0;
            value |= _pointer[ReadPosition++];
            value <<= 8;
            value |= _pointer[ReadPosition++];
            value <<= 8;
            value |= _pointer[ReadPosition++];
            value <<= 8;
            value |= _pointer[ReadPosition++];

            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="int"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="int"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int ReadIntAt(int index)
        {
#if DEBUG
            if (Mode == BufferMode.Read && index + 4 > WritePosition || index + 4 > BufferLimit())
                throw new EndOfStreamException();
#endif

            int value = 0;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index];

            return value;
        }

        /// <summary>
        /// Read current postion as <see cref="int"/>
        /// </summary>
        /// <returns><see cref="uint"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public uint ReadUInt()
        {
            var value = ReadUIntAt(ReadPosition);
            ReadPosition += 4;
            return value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="UInt32"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="uint"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public uint ReadUIntAt(int index)
        {
#if DEBUG
            if (Mode == BufferMode.Read && index + 4 > WritePosition || index + 4 > BufferLimit())
                throw new EndOfStreamException();
#endif

            uint value = 0;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index];

            return value;
        }

        /// <summary>
        /// Read current postion as <see cref="long"/>
        /// </summary>
        /// <returns><see cref="long"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long ReadLong()
        {
            long res =  ReadLongAt(ReadPosition);
            ReadPosition += 8;
            return res;
        }

        /// <summary>
        /// Read at a specific position as <see cref="long"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="long"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long ReadLongAt(int index)
        {
#if DEBUG
            if (Mode == BufferMode.Read && index + 8 > WritePosition || index + 8 > BufferLimit())
                throw new EndOfStreamException();
#endif

            ulong value = 0L;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index];

            return (long)value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="long"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <param name="size">The size of the long in bytes</param>
        /// <returns><see cref="long"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long ReadLongAt(int index, int size)
        {
#if DEBUG
            if (Mode == BufferMode.Read && index + size > WritePosition || index + size > BufferLimit())
                throw new EndOfStreamException();
#endif

            ulong value = 0L;
            for (int i = index; i < index + size; i++)
            {
                value <<= 8;
                value |= _pointer[i];
            }
            return (long)value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="long"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="long"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ulong ReadULongAt(int index)
        {
#if DEBUG
            if (Mode == BufferMode.Read && index + 8 > WritePosition || index + 8 > BufferLimit())
                throw new EndOfStreamException();
#endif

            ulong value = 0L;
            for (int i = index; i < index + 8; i++)
            {
                value <<= 8;
                value |= _pointer[i];
            }

            return value;
        }

        /// <summary>
        /// Read current postion as <see cref="Int16"/>
        /// </summary>
        /// <returns><see cref="int"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public short ReadShort()
        {
#if DEBUG
            if (Mode == BufferMode.Read && ReadPosition + 2 > WritePosition || ReadPosition + 2 > BufferLimit())
                throw new EndOfStreamException();
#endif
            ushort value = 0;
            value |= _pointer[ReadPosition++];
            value <<= 8;
            value |= _pointer[ReadPosition++];

            return (short)value;
        }

        /// <summary>
        /// Read at a specific position as <see cref="short"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="short"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public short ReadShortAt(int index)
        {
#if DEBUG
            if (Mode == BufferMode.Read && index + 2 > WritePosition || index + 2 > BufferLimit())
                throw new EndOfStreamException();
#endif

            ushort value = 0;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index];

            return (short)value;
        }

        /// <summary>
        /// Read current postion as <see cref="ushort"/>
        /// </summary>
        /// <returns><see cref="ushort"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ushort ReadUShort()
        {
            ushort res = ReadUShortAt(ReadPosition);
            ReadPosition += 2;
            return res;
        }

        /// <summary>
        /// Read at a specific position as <see cref="ushort"/>
        /// </summary>
        /// <param name="index">The position to get value</param>
        /// <returns><see cref="ushort"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ushort ReadUShortAt(int index)
        {
#if DEBUG
            if (Mode == BufferMode.Read && index + 2 > WritePosition || index + 2 > BufferLimit())
                throw new EndOfStreamException();
#endif
            ushort value = 0;
            value |= _pointer[index++];
            value <<= 8;
            value |= _pointer[index];

            return value;
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
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Put(byte[] source, int offset, int length)
        {
            int limit = BufferLimit();
            if (WritePosition + length <= limit)
            {
                Buffer.BlockCopy(source, offset, _data, WritePosition, length);
            }
            else
            {
                length = limit - WritePosition;
                Buffer.BlockCopy(source, offset, _data, WritePosition, length);
            }

            WritePosition += length;

            return this;
        }

        /// <summary>
        /// Copy content from a byte array.
        /// </summary>
        /// <param name="source">The byte array to copy content from</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Put(byte[] source)
        {
            return Put(source, 0, source.Length);
        }

        /// <summary>
        /// Copy content from a <see cref="ByteBuffer"/>
        /// </summary>
        /// <param name="sourceBuffer">The <see cref="ByteBuffer"/> to copy content from</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Write(byte value)
        {
            if (WritePosition + 1 <= BufferLimit())
            {
                _pointer[WritePosition] = value;
                WritePosition++;
            }

            return this;
        }

        /// <summary>
        /// Write a <see cref="byte"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="byte"/> value</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer WriteAt(int index, byte value)
        {
            if (index + 1 <= BufferLimit())
            {
                _pointer[index] = value;
            }

            return this;
        }

        /// <summary>
        /// Write a <see cref="double"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="double"/> to write</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Write(double value)
        {
            if (WritePosition + 8 <= BufferLimit())
            {

                Buffer.BlockCopy(BitConverter.GetBytes(value), 0, _data, WritePosition, 8); // BitConverter.GetBytes byte order coincides with the system byte order
#if !BIGENDIAN
                Array.Reverse(_data, WritePosition, 8);
#endif

                WritePosition += 8;
            }
            return this;
        }

        /// <summary>
        /// Write a <see cref="double"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="double"/> value</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer WriteAt(int index, double value)
        {
            if (index + 8 <= BufferLimit())
            {

                Buffer.BlockCopy(BitConverter.GetBytes(value), 0, _data, index, 8); // BitConverter.GetBytes byte order coincides with the system byte order
#if !BIGENDIAN
                Array.Reverse(_data, index, 8);
#endif
            }
            return this;
        }

        /// <summary>
        /// Write a <see cref="float"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="float"/> to write</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Write(float value)
        {
            if (WritePosition + 4 <= BufferLimit())
            {
                Buffer.BlockCopy(BitConverter.GetBytes(value), 0, _data, WritePosition, 4); // BitConverter.GetBytes byte order coincides with the system byte order
#if !BIGENDIAN
                Array.Reverse(_data, WritePosition, 4);
#endif
                WritePosition += 4;
            }
            return this;
        }

        /// <summary>
        /// Write a <see cref="float"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="float"/> value</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer WriteAt(int index, float value)
        {
            if (index + 4 <= BufferLimit())
            {
                Buffer.BlockCopy(BitConverter.GetBytes(value), 0, _data, index, 4); // BitConverter.GetBytes byte order coincides with the system byte order
#if !BIGENDIAN
                Array.Reverse(_data, index, 4);
#endif
            }
            return this;
        }

        /// <summary>
        /// Write a <see cref="Int32"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="Int32"/> to write</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Write(Int32 value)
        {
            int index = WritePosition;
            if (index + 4 <= BufferLimit())
            {
                _pointer[index++] = (byte)((value >> 24) & 0xFF);
                _pointer[index++] = (byte)((value >> 16) & 0xFF);
                _pointer[index++] = (byte)((value >> 8) & 0xFF);
                _pointer[index] = (byte)(value & 0xFF);
                WritePosition += 4;
            }

            return this;
        }

        /// <summary>
        /// Write a <see cref="Int32"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="Int32"/> value</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public unsafe ByteBuffer WriteAt(int index, Int32 value)
        {
            if (index + 4 <= BufferLimit())
            {
                _pointer[index++] = (byte)((value >> 24) & 0xFF);
                _pointer[index++] = (byte)((value >> 16) & 0xFF);
                _pointer[index++] = (byte)((value >> 8) & 0xFF);
                _pointer[index] = (byte)(value & 0xFF);
            }

            return this;
        }

        /// <summary>
        /// Write a <see cref="Int16"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="Int16"/> to write</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Write(Int16 value)
        {
            int index = WritePosition;
            if (index + 2 <= BufferLimit())
            {
                _pointer[index++] = (byte)((value >> 8) & 0xFF);
                _pointer[index] = (byte)(value & 0xFF);
                WritePosition += 2;
            }
            return this;
        }

        /// <summary>
        /// Write a <see cref="Int16"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="Int16"/> value</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public unsafe ByteBuffer WriteAt(int index, short value)
        {
            if (index + 2 <= BufferLimit())
            {
                _pointer[index++] = (byte)((value >> 8) & 0xFF);
                _pointer[index] = (byte)(value & 0xFF);
            }

            return this;
        }

        /// <summary>
        /// Write a <see cref="Int64"/> value to the current position
        /// </summary>
        /// <param name="value">A <see cref="Int64"/> to write</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ByteBuffer Write(long value)
        {
            int index = WritePosition;
            if (index + 8 <= BufferLimit())
            {
                _pointer[index++] = (byte)((value >> 56) & 0xFF);
                _pointer[index++] = (byte)((value >> 48) & 0xFF);
                _pointer[index++] = (byte)((value >> 40) & 0xFF);
                _pointer[index++] = (byte)((value >> 32) & 0xFF);
                _pointer[index++] = (byte)((value >> 24) & 0xFF);
                _pointer[index++] = (byte)((value >> 16) & 0xFF);
                _pointer[index++] = (byte)((value >> 8) & 0xFF);
                _pointer[index] = (byte)(value & 0xFF);
                WritePosition += 8;
            }
            return this;
        }

        /// <summary>
        /// Write a <see cref="Int64"/> value to a specific postion
        /// </summary>
        /// <param name="index">The position to write data to</param>
        /// <param name="value">The <see cref="Int64"/> value</param>
        /// <returns>This object</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public unsafe ByteBuffer WriteAt(int index, long value)
        {
            if (index + 8 <= BufferLimit())
            {
                _pointer[index++] = (byte)((value >> 56) & 0xFF);
                _pointer[index++] = (byte)((value >> 48) & 0xFF);
                _pointer[index++] = (byte)((value >> 40) & 0xFF);
                _pointer[index++] = (byte)((value >> 32) & 0xFF);
                _pointer[index++] = (byte)((value >> 24) & 0xFF);
                _pointer[index++] = (byte)((value >> 16) & 0xFF);
                _pointer[index++] = (byte)((value >> 8) & 0xFF);
                _pointer[index] = (byte)(value & 0xFF);
            }

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

        /// <summary>
        /// Implements Dispose pattern.
        /// Frees the GCHandle which is used to pin underlying byte array 
        /// and create a pointer to the array for better performance.
        /// Call this method if deterministic behavior is needed, 
        /// otherwise the handle will be freed in the finalizer of the class.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
        }

        void Dispose(bool disposing)
        {
            if (disposed)
                return;
            disposed = true;

            if (_handle.IsAllocated) _handle.Free();
            if (!_isDataExternal)
            {
                _data = null;
            }
            if (disposing)
                GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Finalizer for the ByteBuffer class
        /// </summary>
        ~ByteBuffer()
        {
            Dispose(false);
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
            var byteStrings = _data.Select(b => $"{b:X2}");
            string payload = String.Join(", ", byteStrings);
            return $"ReadPosition: {ReadPosition}, WritePosition: {WritePosition}, Limit: {Limit}, Contents[{_data?.Length}]\n{{ {payload} }}";
        }
    }
}
