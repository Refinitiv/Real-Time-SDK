/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Codec
{
	internal class DataBufferWriterWireFormatV1 : BufferWriter
	{
		internal override void Clear()
		{
			_buffer.Clear();
			_reservedBytes = 0;
		}

		internal override byte MajorVersion()
		{
			return RwfDataConstants.MAJOR_VERSION_1;
		}

		internal override byte MinorVersion()
		{
			return RwfDataConstants.MINOR_VERSION_1;
		}

		/* Signed Long - 64 bits */
		internal override CodecReturnCode WriteLong64ls(long v)
		{
			if ((v >= -0x80) && (v < 0x80))
			{
				if (HasRemaining(1))
				{
					WriteByte(unchecked((byte)(v & 0xFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x8000) && (v < 0x8000))
			{
				if (HasRemaining(2))
				{
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x800000) && (v < 0x800000))
			{
				if (HasRemaining(3))
				{
					WriteByte(unchecked((byte)((v >> 16) & 0xFF)));
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x80000000L) && (v < 0x80000000L))
			{
				if (HasRemaining(4))
				{
					WriteUInt(v & 0xFFFFFFFF);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x8000000000L) && (v < 0x8000000000L))
			{
				if (HasRemaining(5))
				{
					WriteByte(unchecked((byte)((v >> 32) & 0xFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x800000000000L) && (v < 0x800000000000L))
			{
				if (HasRemaining(6))
				{
					WriteUShort(unchecked((short)((v >> 32) & 0xFFFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x80000000000000L) && (v < 0x80000000000000L))
			{
				if (HasRemaining(7))
				{
                    sbyte result = (sbyte)(v >> 48);

                    WriteByte((byte)(v >> 48));
					WriteUShort(unchecked((short)((v >> 32) & 0x00FFFF)));
					WriteUInt(v & 0xFFFFFFFFL);

                    return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				if (HasRemaining(8))
				{
					WriteLong(v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
		}

		/* Signed Long - 64 bits */
		internal override CodecReturnCode WriteLong64lsWithLength(long v)
		{
			if ((v >= -0x80) && (v < 0x80))
			{
				if (HasRemaining(2))
				{
					WriteByte(1);
					WriteByte(unchecked((sbyte)(v & 0xFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x8000) && (v < 0x8000))
			{
				if (HasRemaining(3))
				{
					WriteByte(2);
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x800000) && (v < 0x800000))
			{
				if (HasRemaining(4))
				{
					WriteByte(3);
					WriteByte(unchecked((sbyte)((v >> 16) & 0xFF)));
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x80000000L) && (v < 0x80000000L))
			{
				if (HasRemaining(5))
				{
					WriteByte(4);
					WriteUInt(v & 0xFFFFFFFF);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x8000000000L) && (v < 0x8000000000L))
			{
				if (HasRemaining(6))
				{
					WriteByte(5);
					WriteByte(unchecked((sbyte)((v >> 32) & 0xFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x800000000000L) && (v < 0x800000000000L))
			{
				if (HasRemaining(7))
				{
					WriteByte(6);
					WriteUShort(unchecked((short)((v >> 32) & 0xFFFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x80000000000000L) && (v < 0x80000000000000L))
			{
				if (HasRemaining(8))
				{
					WriteByte(7);
					WriteByte(unchecked((sbyte)((v >> 48) & 0xFF)));
					WriteUShort(unchecked((short)((v >> 32) & 0x00FFFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				if (HasRemaining(9))
				{
					WriteByte(8);
					WriteLong(v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
		}

		/* Signed Long - 64 bits */
		internal override CodecReturnCode WriteLong64lsBy2WithLength(long v)
		{
			if ((v >= -0x8000) && (v < 0x8000))
			{
				if (HasRemaining(3))
				{
					WriteByte(2);
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x80000000L) && (v < 0x80000000L))
			{
				if (HasRemaining(5))
				{
					WriteByte(4);
					WriteUInt(v & 0xFFFFFFFF);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if ((v >= -0x800000000000L) && (v < 0x800000000000L))
			{
				if (HasRemaining(7))
				{
					WriteByte(6);
					WriteUShort(unchecked((short)((v >> 32) & 0xFFFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				if (HasRemaining(9))
				{
					WriteByte(8);
					WriteLong(v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
		}

		/* Unsigned Integer - 16 bits */
		internal override CodecReturnCode WriteUInt16ls(int v)
		{
			if (v < 0)
			{
				return CodecReturnCode.INVALID_DATA;
			}

			if (v <= 0xFFL)
			{
				if (HasRemaining(1))
				{
					WriteByte(unchecked((sbyte)(v & 0xFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFL)
			{
				if (HasRemaining(2))
				{
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				return CodecReturnCode.INVALID_DATA;
			}
		}

		/* Unsigned Integer - 16 bits */
		internal override CodecReturnCode WriteUInt16lsWithLength(int v)
		{
			if (v < 0)
			{
				return CodecReturnCode.INVALID_DATA;
			}

			if (v <= 0xFFL)
			{
				if (HasRemaining(2))
				{
					WriteByte(1);
					WriteByte(unchecked((byte)(v & 0xFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFL)
			{
				if (HasRemaining(3))
				{
					WriteByte(2);
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				return CodecReturnCode.INVALID_DATA;
			}
		}

		/* Unsigned Integer - 30 bits of value */
		internal override CodecReturnCode WriteUInt30rb(int v)
		{
			if (v < 0)
			{
				return CodecReturnCode.INVALID_DATA;
			}

			if (v < 0x40)
			{
				if (HasRemaining(1))
				{
					WriteByte(v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v < 0x4000)
			{
				if (HasRemaining(2))
				{
					int v1 = v | 0x8000;
					WriteShort(v1);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v < 0x400000)
			{
				if (HasRemaining(3))
				{
					int v1 = v | 0x400000;
					WriteByte(unchecked((byte)((v1 >> 16) & 0xFF)));
					WriteShort(unchecked((short)(v1 & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v < 0x40000000)
			{
				if (HasRemaining(4))
				{
					int v1 = v | unchecked((int)0xC0000000);
					WriteUInt(v1);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				return CodecReturnCode.INVALID_DATA;
			}
		}

		/* unsigned integer - 32 bits */
		internal override CodecReturnCode WriteUInt32ob(long v)
		{
			if (v < 0)
			{
				return CodecReturnCode.INVALID_DATA;
			}

			if (v < 0xFEL)
			{
				if (HasRemaining(1))
				{
					WriteByte((int)v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFL)
			{
				if (HasRemaining(3))
				{
					WriteByte(0xFE);
					WriteUShort((short)v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFL)
			{
				if (HasRemaining(5))
				{
					WriteByte(0xFF);
					WriteUInt((int)v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				return CodecReturnCode.INVALID_DATA;
			}
		}

		/* Unsigned Long - 64 bits */
		internal override CodecReturnCode WriteULong64ls(long v)
		{
			if (v < 0)
			{
				if (HasRemaining(8))
				{
					WriteLong(v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFL)
			{
				if (HasRemaining(1))
				{
					WriteByte(unchecked((sbyte)(v & 0xFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFL)
			{
				if (HasRemaining(2))
				{
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFL)
			{
				if (HasRemaining(3))
				{
					WriteByte(unchecked((sbyte)((v >> 16) & 0xFF)));
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFL)
			{
				if (HasRemaining(4))
				{
					WriteUInt(v & 0xFFFFFFFF);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFFFL)
			{
				if (HasRemaining(5))
				{
					WriteByte(unchecked((sbyte)((v >> 32) & 0xFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFFFFFL)
			{
				if (HasRemaining(6))
				{
					WriteUShort(unchecked((short)((v >> 32) & 0xFFFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFFFFFFFL)
			{
				if (HasRemaining(7))
				{
					WriteByte(unchecked((sbyte)((v >> 48) & 0xFF)));
					WriteUShort(unchecked((short)((v >> 32) & 0x00FFFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				if (HasRemaining(8))
				{
					WriteLong(v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
		}

		internal override CodecReturnCode WriteULong64lsWithLength(long v)
		{
			if (v < 0)
			{
				if (HasRemaining(9))
				{
					WriteByte(8);
					WriteLong(v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFL)
			{
				if (HasRemaining(2))
				{
					WriteByte(1);
					WriteByte(unchecked((byte)(v & 0xFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFL)
			{
				if (HasRemaining(3))
				{
					WriteByte(2);
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFL)
			{
				if (HasRemaining(4))
				{
					WriteByte(3);
					WriteByte(unchecked((byte)((v >> 16) & 0xFF)));
					WriteUShort(unchecked((short)(v & 0xFFFF)));
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFL)
			{
				if (HasRemaining(5))
				{
					WriteByte(4);
					WriteUInt(v & 0xFFFFFFFF);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFFFL)
			{
				if (HasRemaining(6))
				{
					WriteByte(5);
					WriteByte(unchecked((byte)((v >> 32) & 0xFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFFFFFL)
			{
				if (HasRemaining(7))
				{
					WriteByte(6);
					WriteUShort(unchecked((short)((v >> 32) & 0xFFFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else if (v <= 0xFFFFFFFFFFFFFFL)
			{
				if (HasRemaining(8))
				{
					WriteByte(7);
					WriteByte(unchecked((byte)((v >> 48) & 0xFF)));
					WriteUShort(unchecked((short)((v >> 32) & 0x00FFFF)));
					WriteUInt(v & 0xFFFFFFFFL);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
			else
			{
				if (HasRemaining(9))
				{
					WriteByte(8);
					WriteLong(v);
					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
			}
		}

		/*
		 * This method combined with the calling code replaces the previous writeUShort15rb method.
		 * If the value v is smaller than 0x80 the value is written on the wire as one byte, otherwise it is two bytes.
		 * The calling code must check the value and ensure that the buffer is sufficient enough to write this value (either one or two bytes).
		 * If the value is smaller than 0x80 the calling code uses writeByte method.
		 * This method is called only when the v >= 0x80.
		 */
		internal override void WriteUShort15rbLong(short v)
		{
			// the v is verified in the preceding code to be >=0
			short v1 = unchecked((short)(v | 0x8000));
			WriteShort(v1);
		}

        /*
		 * This method combined with the calling code replaces the previous writeUShort16ob method.
		 * If the value v is smaller than 0xFE the value is written on the wire as one byte, otherwise it is three bytes.
		 * The calling code must check the value and ensure that the buffer is sufficient enough
		 * to write this value (either one or three bytes).
		 * If the value is smaller than 0xFE the calling code uses writeByte method.
		 * This method is called only when the v >= 0xFE.
		 */
        internal override void WriteUShort16obLong(int v)
		{
			// the v is verified in the preceding code to be >= 0
			WriteByte(0xFE);
			WriteUShort((short)v);
		}

		internal override void Write(Buffer buf)
		{
			if (buf.Length > 0)
			{
				string bufAsString = buf.DataString();
				if (!string.ReferenceEquals(bufAsString, null))
				{
					// copy ASCII data (8bits).
					int pos = buf.Position;
					for (int idx = 0; idx < buf.Length; idx++)
					{
						_buffer.Write(unchecked((byte)(0xff & bufAsString[idx + pos])));
					}
				}
				else
				{
					ByteBuffer byteBuffer = buf.Data();
					_buffer.Put(byteBuffer.Contents, buf.Position, buf.Length);
				}
			}
		}

		internal override CodecReturnCode WriteBytes(string s, string charset)
		{
			try
			{
                System.Text.Encoding encoding = System.Text.Encoding.GetEncoding(charset);
                byte[] bytes = new byte[encoding.GetByteCount(s)];
                encoding.GetBytes(s, 0, s.Length, bytes, encoding.GetByteCount(s));
				_buffer.Put(bytes);
			}
			catch (Exception)
			{
				return CodecReturnCode.INVALID_DATA;
			}
			return CodecReturnCode.SUCCESS;
		}

		internal override void WriteUByte(int v)
		{
			_buffer.Write((byte)v);
		}

		internal override void WriteUShort(int v)
		{
			_buffer.Write((short)v);
		}

		internal override void WriteUInt(long v)
		{
			_buffer.Write((int)v);
		}

		internal override void WriteBoolean(bool v)
		{
			if (v)
			{
				WriteByte(1);
			}
			else
			{
				WriteByte(0);
			}
		}

		internal override void WriteByte(int v)
		{
			_buffer.Write((byte)v);
		}

	

		internal override void WriteDouble(double v)
		{
			_buffer.Write(v);
		}

		internal override void WriteFloat(float v)
		{
			_buffer.Write(v);
		}

		internal override void WriteInt(int v)
		{
			_buffer.Write(v);
		}

		internal override void WriteLong(long v)
		{
			_buffer.Write(v);
		}

		internal override void WriteShort(int v)
		{
            _buffer.Write((short)v);
        }

        // this is reading method, added for utilities
        internal override short GetUShort15rb
		{
			get
			{
                short val = (short)_buffer.ReadByte();
				if (val < 0)
				{
					val &= 0xFF;
				}
    
				if ((val & 0x80) != 0)
				{
                    short b = (short)_buffer.ReadByte();
					if (b < 0)
					{
						b &= 0xFF;
					}
					return (short)(((val & 0x7F) << 8) + b);
				}
    
				return (short)val;
			}
		}

    }

}