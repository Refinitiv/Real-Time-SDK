/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Codec
{
	/* All of the reads are absolute.
	 * The internal buffer's position is not changed.
	 * This reader keeps track of the position.
	 */
	internal abstract class BufferReader
	{
		internal ByteBuffer _buffer;
		internal int _position;

		internal virtual void Data(ByteBuffer buffer)
		{
			_buffer = buffer;
		}

		internal virtual ByteBuffer Buffer()
		{
			return _buffer;
		}

		internal virtual int Position()
		{
			return _position;
		}

		internal virtual void Position(int pos)
		{
			_position = pos;
            Debug.Assert(pos <= _buffer.Limit, "Position beyond limit");
		}

		internal virtual int ReadShort()
		{
			short s = _buffer.ReadShortAt(_position);
			_position += 2; // short
			return s;
		}

		internal virtual sbyte ReadByte()
		{
			sbyte b = (sbyte)_buffer.Contents[_position];
			_position += 1; // byte
			return b;
		}

		internal virtual int ReadInt()
		{
			int i = _buffer.ReadIntAt(_position);
			_position += 4; // int
			return i;
		}

		internal virtual long ReadLong()
		{
			long val = _buffer.ReadLongAt(_position);
			_position += 8; // long
			return val;
		}

		internal virtual double ReadDouble()
		{
            byte[] tmpData = BitConverter.GetBytes(ReadLong());
            double dbl = BitConverter.ToDouble(tmpData, 0);
            return dbl;
		}

		internal virtual float ReadFloat()
		{
            byte[] tmpData = BitConverter.GetBytes(ReadInt());
            float flt = BitConverter.ToSingle(tmpData, 0);
            return flt;
		}

		internal virtual bool ReadBoolean()
		{
			int val = ReadUnsignedByte();
			return val != 0;
		}

		internal virtual int ReadUnsignedByte()
		{
			short val = ReadByte();
			if (val < 0)
			{
				val &= 0xFF;
			}
			return val;
		}
        
		internal virtual int ReadUnsignedShort()
		{
			int val = ReadShort();

			if (val < 0)
			{
				val &= 0xFFFF;
			}

			return val;
		}
        
		internal virtual long ReadUnsignedInt()
		{
			long val = ReadInt();
			if (val < 0)
			{
				val &= 0xFFFFFFFFL;
			}
			return val;
		}

		internal virtual long ReadULong()
		{
			long val = ReadLong();
			return val;
		}
        
		internal virtual void SkipBytes(int i)
		{
			_position += i;
            Debug.Assert(_position <= _buffer.Limit, "Position is out of range");
		}

		/* Relative read of an unsigned 32 bit number. */
		internal virtual long ReadRelativeUnsignedInt()
		{
			long val = _buffer.ReadInt();
			if (val < 0)
			{
				val &= 0xFFFFFFFFL;
			}
			return val;
		}
        
		internal abstract short ReadUShort15rb();
		internal abstract int ReadUShort16ob();
		internal abstract int ReadInt16ls(int size);
		internal abstract int ReadInt32ls(int size);
		internal abstract ushort ReadUInt16ls(int size);
		internal abstract int ReadUInt30rb();
		internal abstract long ReadUInt32ob();
		internal abstract long ReadUInt32ls(int size);
		internal abstract long ReadLong64ls(int size);
		internal abstract long ReadULong64ls(int size);
		internal abstract byte MajorVersion();
		internal abstract byte MinorVersion();
		internal abstract void Clear();
		internal abstract short ReadRelativeUShort15rb();
        internal abstract int ReadRelativeUShort16ob();
	}

}