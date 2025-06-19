/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
using LSEG.Eta.Common;

namespace LSEG.Eta.Codec
{
	/* All of the reads are absolute.
	 * The internal buffer's position is not changed.
	 * This reader keeps track of the position.
	 */
	sealed internal unsafe class BufferReader
	{
		internal ByteBuffer _buffer;
		internal int _position;
		internal byte* pointer = null;

		static readonly ulong[] num = { 0xFFFFFFFFFFFFFFFF , 0xFFFFFFFFFFFFFFFE , 0xFFFFFFFFFFFFFFFC , 0xFFFFFFFFFFFFFFF8 ,
						0xFFFFFFFFFFFFFFF0 , 0xFFFFFFFFFFFFFFE0 , 0xFFFFFFFFFFFFFFC0 , 0xFFFFFFFFFFFFFF80 ,
						0xFFFFFFFFFFFFFF00 , 0xFFFFFFFFFFFFFE00 , 0xFFFFFFFFFFFFFC00 , 0xFFFFFFFFFFFFF800 ,
						0xFFFFFFFFFFFFF000 , 0xFFFFFFFFFFFFE000 , 0xFFFFFFFFFFFFC000 , 0xFFFFFFFFFFFF8000 ,
						0xFFFFFFFFFFFF0000 , 0xFFFFFFFFFFFE0000 , 0xFFFFFFFFFFFC0000 , 0xFFFFFFFFFFF80000 ,
						0xFFFFFFFFFFF00000 , 0xFFFFFFFFFFE00000 , 0xFFFFFFFFFFC00000 , 0xFFFFFFFFFF800000 ,
						0xFFFFFFFFFF000000 , 0xFFFFFFFFFE000000 , 0xFFFFFFFFFC000000 , 0xFFFFFFFFF8000000 ,
						0xFFFFFFFFF0000000 , 0xFFFFFFFFE0000000 , 0xFFFFFFFFC0000000 , 0xFFFFFFFF80000000 ,
						0xFFFFFFFF00000000 , 0xFFFFFFFE00000000 , 0xFFFFFFFC00000000 , 0xFFFFFFF800000000 ,
						0xFFFFFFF000000000 , 0xFFFFFFE000000000 , 0xFFFFFFC000000000 , 0xFFFFFF8000000000 ,
						0xFFFFFF0000000000 , 0xFFFFFE0000000000 , 0xFFFFFC0000000000 , 0xFFFFF80000000000 ,
						0xFFFFF00000000000 , 0xFFFFE00000000000 , 0xFFFFC00000000000 , 0xFFFF800000000000 ,
						0xFFFF000000000000 , 0xFFFE000000000000 , 0xFFFC000000000000 , 0xFFF8000000000000 ,
						0xFFF0000000000000 , 0xFFE0000000000000 , 0xFFC0000000000000 , 0xFF80000000000000 ,
						0xFF00000000000000 , 0xFE00000000000000 , 0xFC00000000000000 , 0xF800000000000000 ,
						0xF000000000000000 , 0xE000000000000000 , 0xC000000000000000 , 0x8000000000000000 , 
						0};

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal void Data(ByteBuffer buffer)
		{
            _buffer = buffer;
			pointer = buffer._pointer;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal ByteBuffer Buffer()
		{
			return _buffer;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int Position()
		{
			return _position;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal void Position(int pos)
		{
			_position = pos;
            Debug.Assert(pos <= _buffer.Limit, "Position beyond limit");
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadShort()
		{
#if DEBUG
			if (_buffer.Mode == ByteBuffer.BufferMode.Read && _position + 2 > _buffer.WritePosition || _position + 2 > _buffer.BufferLimit())
				throw new EndOfStreamException();
#endif
			ushort value = 0;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			return (short)value;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal int ReadUShort()
		{
#if DEBUG
			if (_buffer.Mode == ByteBuffer.BufferMode.Read && _position + 2 > _buffer.WritePosition || _position + 2 > _buffer.BufferLimit())
				throw new EndOfStreamException();
#endif
			ushort value = 0;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			return value;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal sbyte ReadByte()
		{
			return (sbyte)pointer[_position++];
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadInt()
		{
#if DEBUG
			if (_buffer.Mode == ByteBuffer.BufferMode.Read && _position + 4 > _buffer.WritePosition || _position + 4 > _buffer.BufferLimit())
				throw new EndOfStreamException();
#endif
			int value = 0;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			return value;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal long ReadLong()
		{
#if DEBUG
			if (_buffer.Mode == ByteBuffer.BufferMode.Read && _position + 8 > _buffer.WritePosition || _position + 8 > _buffer.BufferLimit())
				throw new EndOfStreamException();
#endif

			ulong value = 0L;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];

			return (long)value;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal double ReadDouble()
		{
			byte[] tmpData = BitConverter.GetBytes(ReadLong());
            double dbl = BitConverter.ToDouble(tmpData, 0);
            return dbl;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal float ReadFloat()
		{
			byte[] tmpData = BitConverter.GetBytes(ReadInt());
            float flt = BitConverter.ToSingle(tmpData, 0);
            return flt;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal bool ReadBoolean()
		{
			return pointer[_position++] != 0;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadUnsignedByte()
		{
			return pointer[_position++];
		}
        
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal ushort ReadUnsignedShort()
		{
#if DEBUG
			if (_buffer.Mode == ByteBuffer.BufferMode.Read && _position + 2 > _buffer.WritePosition || _position + 2 > _buffer.BufferLimit())
				throw new EndOfStreamException();
#endif

			ushort value = 0;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			return value;
		}
        
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal uint ReadUnsignedInt()
		{
#if DEBUG
			if (_buffer.Mode == ByteBuffer.BufferMode.Read && _position + 4 > _buffer.WritePosition || _position + 4 > _buffer.BufferLimit())
				throw new EndOfStreamException();
#endif

			uint value = 0;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];
			value <<= 8;
			value |= pointer[_position++];

			return value;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal long ReadULong()
		{
			long res = _buffer.ReadLongAt(_position);
			_position += 8;
			return res;
		}
        
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal void SkipBytes(int i)
		{
			_position += i;
            Debug.Assert(_position <= _buffer.Limit, "Position is out of range");
		}

		/* Relative read of an unsigned 32 bit number. */
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal uint ReadRelativeUnsignedInt()
		{
#if DEBUG
			if (_buffer.Mode == ByteBuffer.BufferMode.Read && _buffer.ReadPosition + 4 > _buffer.WritePosition || _buffer.ReadPosition + 4 > _buffer.BufferLimit())
				throw new EndOfStreamException();
#endif

			uint value = 0;
			value |= pointer[_buffer.ReadPosition++];
			value <<= 8;
			value |= pointer[_buffer.ReadPosition++];
			value <<= 8;
			value |= pointer[_buffer.ReadPosition++];
			value <<= 8;
			value |= pointer[_buffer.ReadPosition++];

			return value;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal void Clear()
		{
			_buffer = null;
			pointer = null;
			_position = 0;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal byte MajorVersion()
		{
			return RwfDataConstants.MAJOR_VERSION_1;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal byte MinorVersion()
		{
			return RwfDataConstants.MINOR_VERSION_1;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal ushort ReadUShort15rb()
		{
			ushort b = pointer[_position++];
			if ((b & 0x80) != 0)
			{
				return (ushort)(((b & 0x7F) << 8) | pointer[_position++]);
			}

			return b;

		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadUShort16ob()
		{
			ushort b = pointer[_position++];
			if (b == 0xFE)
			{
				b = pointer[_position++];
				b <<= 8;
				b |= pointer[_position++];
			}
			return b;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadInt16ls(int size)
		{
			switch (size)
			{
				case 0:
					return 0;
				case 1:
					return ReadByte();
				case 2:
					return ReadShort();
				default:
					return 0;
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadInt32ls(int size)
		{
			switch (size)
			{
				case 0:
					return 0;
				case 1:
					return ReadByte();
				case 2:
					return ReadShort();
				case 3:
					return (ReadByte() << 16) | ReadUnsignedShort();
				case 4:
					int res = _buffer.ReadIntAt(_position);
					_position += 4;
					return res;
				default:
					return 0;
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal ushort ReadUInt16ls(int size)
		{
			switch (size)
			{
				case 0:
					return 0;
				case 1:
					return pointer[_position++];
				case 2:
#if DEBUG
					if (_position + 2 > _buffer.BufferLimit())
						throw new EndOfStreamException();
#endif
					ushort value = 0;
					value |= pointer[_position++];
					value <<= 8;
					value |= pointer[_position++];
					return value;

				default:
					return 0;
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadUInt30rb()
		{
			int b = pointer[_position++];
			int bitflags = b & 0xC0;

			if (bitflags == 0)
			{
				return b;
			}
			else if (bitflags == 0x80)
			{
				return ((b & 0x3F) << 8) | pointer[_position++];
			}
			else if (bitflags == 0x40)
			{
				return ((b & 0x3F) << 16) | ReadUnsignedShort();
			}
			else // (bitflags == 0xC0)
			{
				return ((b & 0x3F) << 24) | (pointer[_position++] << 16) | ReadUnsignedShort();
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal long ReadUInt32ls(int size)
		{
			switch (size)
			{
				case 0:
					return 0;
				case 1:
					return pointer[_position++];
				case 2:
					return pointer[_position++];
				case 3:
					return (pointer[_position++] << 16) | ReadUnsignedShort();
				case 4:
					return ReadUnsignedInt();
				default:
					return 0;
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal long ReadUInt32ob()
		{
			int b = ReadUnsignedByte();
			if (b == 0xFE)
			{
				return ReadUnsignedShort();
			}
			else if (b == 0xFF)
			{
				return ReadUnsignedInt();
			}
			else
			{
				return b;
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal long ReadLong64ls(int size)
		{
		    long value = 0L;
			for (int i = _position; i < _position + size; i++)
			{
				value <<= 8;
				value |= pointer[i];
			}
			if ((pointer[_position] & 128) == 128) //sign-extend
			{
				value = (long)((ulong)value | num[size * 8]);
			}
			_position += size;

			return value;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal ulong ReadULong64ls(int size)
		{
			ulong value = 0;
			for (int i = _position; i < _position + size; i++)
			{
				value <<= 8;
				value |= pointer[i];
			}
			_position += size;

			return value;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadRelativeUShort15rb()
		{
			byte val = pointer[_buffer.ReadPosition];
			_buffer.ReadPosition++;

			if ((val & 0x80) != 0)
			{
				byte b = pointer[_buffer.ReadPosition];
				_buffer.ReadPosition++;
				return ((val & 0x7F) << 8) | b;
			}

			return val;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)] 
		internal int ReadRelativeUShort16ob()
		{
			ushort val = pointer[_buffer.ReadPosition++];

			if (val == 0xFE)
			{
#if DEBUG
				if (_buffer.Mode == ByteBuffer.BufferMode.Read && _buffer.ReadPosition + 2 > _buffer.WritePosition || _buffer.ReadPosition + 2 > _buffer.BufferLimit())
					throw new EndOfStreamException();
#endif
				val |= pointer[_buffer.ReadPosition++];
				val <<= 8;
				val |= pointer[_buffer.ReadPosition++];
			}
			return val;
		}
	}

}
