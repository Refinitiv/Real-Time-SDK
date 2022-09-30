/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{

	/* All ByteBuffer reads are absolute and are done in BufferReader. */
	internal class DataBufferReaderWireFormatV1 : BufferReader
	{
		internal override void Clear()
		{
			_buffer = null;
			_position = 0;
		}

		internal override byte MajorVersion()
		{
			return RwfDataConstants.MAJOR_VERSION_1;
		}

		internal override byte MinorVersion()
		{
			return RwfDataConstants.MINOR_VERSION_1;
		}

		internal override short ReadUShort15rb()
		{
			int b = ReadUnsignedByte();
			if ((b & 0x80) != 0)
			{
				return (short)(((b & 0x7F) << 8) + ReadUnsignedByte());
			}

			return (short)b;
		}

		internal override int ReadUShort16ob()
		{
			int b = ReadUnsignedByte();
			if (b == 0xFE)
			{
				return ReadUnsignedShort();
			}

			return b;
		}

		internal override int ReadInt16ls(int size)
		{
			switch (size)
			{
				case 0:
					return 0;
				case 1:
					return ReadByte();
				case 2:
				{
					return ReadShort();
				}
				default:
                    return 0;
			}
		}

		internal override int ReadInt32ls(int size)
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
					return ReadInt();
				default:
                    return 0;
			}
		}

		internal override ushort ReadUInt16ls(int size)
		{
			switch (size)
			{
				case 0:
					return 0;
				case 1:
					return (ushort)ReadUnsignedByte();
				case 2:
					return (ushort)ReadUnsignedShort();
				default:
					return 0;
			}
		}

		internal override int ReadUInt30rb()
		{
			int b = ReadUnsignedByte();
			int bitflags = b & 0xC0;

			if (bitflags == 0)
			{
				return b;
			}
			else if (bitflags == 0x80)
			{
				return ((b & 0x3F) << 8) + ReadUnsignedByte();
			}
			else if (bitflags == 0x40)
			{
				return ((b & 0x3F) << 16) + ReadUnsignedShort();
			}
			else // (bitflags == 0xC0)
			{
				return ((b & 0x3F) << 24) + (ReadUnsignedByte() << 16) + ReadUnsignedShort();
			}
		}

		internal override long ReadUInt32ls(int size)
		{
			switch (size)
			{
				case 0:
					return 0;
				case 1:
					return ReadUnsignedByte();
				case 2:
					return ReadUnsignedShort();
				case 3:
					return (ReadUnsignedByte() << 16) + ReadUnsignedShort();
				case 4:
					return ReadUnsignedInt();
				default:
					return 0;
			}
		}

		internal override long ReadUInt32ob()
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

		internal override long ReadLong64ls(int size)
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
					return (ReadByte() << 16) + ReadUnsignedShort();
				case 4:
					return ReadInt();
				case 5:
					return (((long)(ReadByte())) << 32) + ReadUnsignedInt();
				case 6:
					return (((long)(ReadShort())) << 32) + ReadUnsignedInt();
				case 7:
                    return (((long)(ReadByte())) << 48) + (((long)(ReadUnsignedShort())) << 32) + ReadUnsignedInt();
				case 8:
					return ReadLong();
				default:
					return 0;
			}
		}

		internal override long ReadULong64ls(int size)
		{
			switch (size)
			{
				case 0:
					return 0;
				case 1:
					return ReadByte() & 0xFF;
				case 2:
					return ReadUnsignedShort();
				case 3:
					return (ReadUnsignedByte() << 16) + (ReadUnsignedShort() & 0xFFFF);
				case 4:
					return ReadUnsignedInt();
				case 5:
					return ((((long)(ReadUnsignedByte())) & 0xFF) << 32) + (ReadUnsignedInt() & 0xFFFFFFFF);
				case 6:
					return (((long)(ReadUnsignedShort())) << 32) + (ReadUnsignedInt() & 0xFFFFFFFF);
				case 7:
					return (((long)ReadUnsignedByte()) << 48) + (((long)ReadUnsignedShort()) << 32) + (ReadUnsignedInt() & 0xFFFFFFFF);
				case 8:
					return ReadLong();
				default:
					return 0;
			}
		}

		internal override short ReadRelativeUShort15rb()
		{
            short val = (short)_buffer.ReadByte();
			if (val < 0)
			{
				val &= 0xFF;
			}

			if ((val & 0x80) != 0)
			{
				byte b = (byte)_buffer.ReadByte();
				if (b < 0)
				{
					b &= unchecked((byte)0xFF);
				}
				return (short)(((val & 0x7F) << 8) + b);
			}

			return (short)val;
		}

		internal override int ReadRelativeUShort16ob()
		{
			short val = (short)_buffer.ReadByte();
			if (val < 0)
			{
				val &= 0xFF;
			}

			if (val == 0xFE)
			{
                val = (short)_buffer.ReadByte();

				if (val < 0)
				{
					val &= unchecked((short)0xFFFF);
				}
			}
			return val;
		}

	}

}