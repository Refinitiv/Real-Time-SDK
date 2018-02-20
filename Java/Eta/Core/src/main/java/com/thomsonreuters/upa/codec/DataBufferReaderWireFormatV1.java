package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.RwfDataConstants;

/* All ByteBuffer reads are absolute and are done in BufferReader. */
class DataBufferReaderWireFormatV1 extends BufferReader 
{
    void clear()
    {
        _buffer = null;
        _position = 0;
    }

    byte majorVersion()
    {
        return RwfDataConstants.MAJOR_VERSION_1;
    }

    byte minorVersion()
    {
        return RwfDataConstants.MINOR_VERSION_1;
    }

    short readUShort15rb() throws Exception
    {
        int b = readUnsignedByte();
        if ((b & 0x80) != 0)
        {
            return (short)(((b & 0x7F) << 8) + readUnsignedByte());
        }

        return (short)b;
    }

    int readUShort16ob() throws Exception
    {
        int b = readUnsignedByte();
        if (b == 0xFE)
        {
            return readUnsignedShort();
        }

        return b;
    }

    int readInt16ls(int size) throws Exception
    {
        switch (size)
        {
            case 0:
                return 0;
            case 1:
                return readByte();
            case 2:
            {
                return readShort();
            }
            default:
                assert (false);
                return 0;
        }
    }

    int readInt32ls(int size) throws Exception
    {
        switch (size)
        {
            case 0:
                return 0;
            case 1:
                return readByte();
            case 2:
                return readShort();
            case 3:
                return (readByte() << 16) | readUnsignedShort();
            case 4:
                return readInt();
            default:
                assert (false);
                return 0;
        }
    }

    int readUInt16ls(int size) throws Exception
    {
        switch (size)
        {
            case 0:
                return 0;
            case 1:
                return readUnsignedByte();
            case 2:
                return readUnsignedShort();
            default:
                assert (false);
                return 0;
        }
    }

    int readUInt30rb() throws Exception
    {
        int b = readUnsignedByte();
        int bitflags = b & 0xC0;

        if (bitflags == 0)
        {
            return b;
        }
        else if (bitflags == 0x80)
        {
            return ((b & 0x3F) << 8) + readUnsignedByte();
        }
        else if (bitflags == 0x40)
        {
            return ((b & 0x3F) << 16) + readUnsignedShort();
        }
        else // (bitflags == 0xC0)
        {
            return ((b & 0x3F) << 24) + (readUnsignedByte() << 16) + readUnsignedShort();
        }
    }

    long readUInt32ls(int size) throws Exception
    {
        switch (size)
        {
            case 0:
                return 0;
            case 1:
                return readUnsignedByte();
            case 2:
                return readUnsignedShort();
            case 3:
                return (readUnsignedByte() << 16) + readUnsignedShort();
            case 4:
                return readUnsignedInt();
            default:
                assert (false);
                return 0;
        }
    }

    long readUInt32ob() throws Exception
    {
        int b = readUnsignedByte();
        if (b == 0xFE)
        {
            return readUnsignedShort();
        }
        else if (b == 0xFF)
        {
            return readUnsignedInt();
        }
        else
            return b;
    }

    long readLong64ls(int size) throws Exception
    {
        switch (size)
        {
            case 0:
                return 0;
            case 1:
                return readByte();
            case 2:
                return readShort();
            case 3:
                return (readByte() << 16) + readUnsignedShort();
            case 4:
                return readInt();
            case 5:
                return (((long)(readByte())) << 32) + readUnsignedInt();
            case 6:
                return (((long)(readShort())) << 32) + readUnsignedInt();
            case 7:
                return (((long)(readByte())) << 48) + (((long)(readUnsignedShort())) << 32) + readUnsignedInt();
            case 8:
                return readLong();
            default:
                assert (false);
                return 0;
        }
    }
    
    // before calling this method make sure 0 <= size <= 8
    long readULong64ls(int size) throws Exception
    {
        switch (size)
        {
            case 0:
                return 0;
            case 1:
                return readByte() & 0xFF;
            case 2:
                return readUnsignedShort();
            case 3:
                return (readUnsignedByte() << 16) + (readUnsignedShort() & 0xFFFF);
            case 4:
                return readUnsignedInt();
            case 5:
                return ((((long)(readUnsignedByte())) & 0xFF) << 32) + (readUnsignedInt() & 0xFFFFFFFF);
            case 6:
                return (((long)(readUnsignedShort())) << 32) + (readUnsignedInt() & 0xFFFFFFFF);
            case 7:
                return (((long)readUnsignedByte()) << 48) + (((long)readUnsignedShort()) << 32) + (readUnsignedInt() & 0xFFFFFFFF);
            case 8:
                return readLong();
            default:
                assert (false);
                return 0;
        }
    }

    @Override
    short readRelativeUShort15rb()
    {
        short val = _buffer.get();
        if (val < 0)
            val &= 0xFF;

        if ((val & 0x80) != 0)
        {
            byte b = _buffer.get();
            if (b < 0)
                b &= 0xFF;
            return (short)(((val & 0x7F) << 8) + b);
        }

        return (short)val;
    }

    @Override
    int readRelativeUShort16ob()
    {
        short val = _buffer.get();
        if (val < 0)
            val &= 0xFF;

        if (val == 0xFE)
        {
            val = _buffer.getShort();

            if (val < 0)
                val &= 0xFFFF;
        }
        return val;
    }

}
