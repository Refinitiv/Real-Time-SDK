/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Codec
{

	internal abstract class BufferWriter
	{
		internal ByteBuffer _buffer;

		// Bytes reserved for data that will need to be written later
		// (e.g. count for a container's entries after summary data is encoded).
		internal int _reservedBytes;

		internal virtual ByteBuffer Buffer()
		{
			return _buffer;
		}

		internal virtual int Position()
		{
			return (int)_buffer.WritePosition;
		}

		internal virtual void Position(int pos)
		{
			_buffer.WritePosition =pos;
		}

		internal virtual void ReserveBytes(int bytes)
		{
			_reservedBytes += bytes;
		}

		internal virtual void UnreserveBytes(int bytes)
		{
			_reservedBytes -= bytes;
		}

		internal virtual bool HasRemaining(int v)
		{
			return _buffer.Remaining - _reservedBytes >= v;
		}

		internal virtual void SkipBytes(int v)
		{
			_buffer.WritePosition = _buffer.Position + v;
		}

		internal abstract void Write(Buffer buf);
		internal abstract void WriteBoolean(bool v);
		internal abstract void WriteByte(int v);
		internal abstract void WriteUByte(int v);
        internal abstract void WriteShort(int v);
        internal abstract void WriteUShort(int v);
		internal abstract void WriteInt(int v);
		internal abstract void WriteUInt(long v);
		internal abstract void WriteLong(long v);
		internal abstract void WriteFloat(float v);
		internal abstract void WriteDouble(double v);
		internal abstract CodecReturnCode WriteBytes(string s, string charset);
		internal abstract CodecReturnCode WriteUInt32ob(long v);
		internal abstract void WriteUShort16obLong(int v);
        internal abstract void WriteUShort15rbLong(short v);
        internal abstract CodecReturnCode WriteUInt30rb(int v);
		internal abstract CodecReturnCode WriteUInt16ls(int v);
		internal abstract CodecReturnCode WriteUInt16lsWithLength(int v);
        internal abstract CodecReturnCode WriteLong64ls(long v);
        internal abstract CodecReturnCode WriteLong64lsWithLength(long v);
		internal abstract CodecReturnCode WriteLong64lsBy2WithLength(long v);
		internal abstract CodecReturnCode WriteULong64ls(long v);
		internal abstract CodecReturnCode WriteULong64lsWithLength(long v);
		internal abstract byte MajorVersion();
		internal abstract byte MinorVersion();
		internal abstract void Clear();

		// This is a reading method, added for utilities.
		// The method returns UShort15rb read at the buffer position.
		internal abstract short GetUShort15rb {get;}
    }

}