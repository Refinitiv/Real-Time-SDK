package com.rtsdk.eta.codec;

import java.nio.ByteBuffer;

import com.rtsdk.eta.codec.RwfDataConstants;

class DataBufferWriterWireFormatV1 extends BufferWriter
{
    void clear()
    {
        _buffer.clear();
        _reservedBytes = 0;
    }

    byte majorVersion()
    {
        return RwfDataConstants.MAJOR_VERSION_1;
    }

    byte minorVersion()
    {
        return RwfDataConstants.MINOR_VERSION_1;
    }

    /* Signed Long - 64 bits */
    int writeLong64ls(long v)
    {
        if ((v >= -0x80) && (v < 0x80))
        {
            if (hasRemaining(1))
            {
                writeByte((byte)(v & 0xFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x8000) && (v < 0x8000))
        {
            if (hasRemaining(2))
            {
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x800000) && (v < 0x800000))
        {
            if (hasRemaining(3))
            {
                writeByte((byte)((v >> 16) & 0xFF));
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x80000000L) && (v < 0x80000000L))
        {
            if (hasRemaining(4))
            {
                writeUInt(v & 0xFFFFFFFF);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x8000000000L) && (v < 0x8000000000L))
        {
            if (hasRemaining(5))
            {
                writeByte((byte)((v >> 32) & 0xFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x800000000000L) && (v < 0x800000000000L))
        {
            if (hasRemaining(6))
            {
                writeUShort((short)((v >> 32) & 0xFFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x80000000000000L) && (v < 0x80000000000000L))
        {
            if (hasRemaining(7))
            {
                writeByte((byte)((v >> 48) & 0xFF));
                writeUShort((short)((v >> 32) & 0x00FFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
        {
            if (hasRemaining(8))
            {
                writeLong(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
    }

    /* Signed Long - 64 bits */
    int writeLong64lsWithLength(long v)
    {
        if ((v >= -0x80) && (v < 0x80))
        {
            if (hasRemaining(2))
            {
                writeByte(1);
                writeByte((byte)(v & 0xFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x8000) && (v < 0x8000))
        {
            if (hasRemaining(3))
            {
                writeByte(2);
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x800000) && (v < 0x800000))
        {
            if (hasRemaining(4))
            {
                writeByte(3);
                writeByte((byte)((v >> 16) & 0xFF));
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x80000000L) && (v < 0x80000000L))
        {
            if (hasRemaining(5))
            {
                writeByte(4);
                writeUInt(v & 0xFFFFFFFF);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x8000000000L) && (v < 0x8000000000L))
        {
            if (hasRemaining(6))
            {
                writeByte(5);
                writeByte((byte)((v >> 32) & 0xFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x800000000000L) && (v < 0x800000000000L))
        {
            if (hasRemaining(7))
            {
                writeByte(6);
                writeUShort((short)((v >> 32) & 0xFFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x80000000000000L) && (v < 0x80000000000000L))
        {
            if (hasRemaining(8))
            {
                writeByte(7);
                writeByte((byte)((v >> 48) & 0xFF));
                writeUShort((short)((v >> 32) & 0x00FFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
        {
            if (hasRemaining(9))
            {
                writeByte(8);
                writeLong(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
    }

    /* Signed Long - 64 bits */
    int writeLong64lsBy2WithLength(long v)
    {
        if ((v >= -0x8000) && (v < 0x8000))
        {
            if (hasRemaining(3))
            {
                writeByte(2);
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x80000000L) && (v < 0x80000000L))
        {
            if (hasRemaining(5))
            {
                writeByte(4);
                writeUInt(v & 0xFFFFFFFF);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x800000000000L) && (v < 0x800000000000L))
        {
            if (hasRemaining(7))
            {
                writeByte(6);
                writeUShort((short)((v >> 32) & 0xFFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
        {
            if (hasRemaining(9))
            {
                writeByte(8);
                writeLong(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
    }

    /* Unsigned Integer - 16 bits */
    int writeUInt16ls(int v)
    {
        if (v < 0)
            return CodecReturnCodes.INVALID_DATA;

        if (v <= 0xFFL)
        {
            if (hasRemaining(1))
            {
                writeByte((byte)(v & 0xFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFL)
        {
            if (hasRemaining(2))
            {
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
            return CodecReturnCodes.INVALID_DATA;
    }

    /* Unsigned Integer - 16 bits */
    int writeUInt16lsWithLength(int v)
    {
        if (v < 0)
            return CodecReturnCodes.INVALID_DATA;

        if (v <= 0xFFL)
        {
            if (hasRemaining(2))
            {
                writeByte(1);
                writeByte((byte)(v & 0xFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFL)
        {
            if (hasRemaining(3))
            {
                writeByte(2);
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
            return CodecReturnCodes.INVALID_DATA;
    }

    /* Unsigned Integer - 30 bits of value */
    int writeUInt30rb(int v)
    {
        if (v < 0)
            return CodecReturnCodes.INVALID_DATA;

        if (v < 0x40)
        {
            if (hasRemaining(1))
            {
                writeByte(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v < 0x4000)
        {
            if (hasRemaining(2))
            {
                int v1 = v | 0x8000;
                writeShort(v1);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v < 0x400000)
        {
            if (hasRemaining(3))
            {
                int v1 = v | 0x400000;
                writeByte((byte)((v1 >> 16) & 0xFF));
                writeShort((short)(v1 & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v < 0x40000000)
        {
            if (hasRemaining(4))
            {
                int v1 = v | 0xC0000000;
                writeUInt(v1);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
            return CodecReturnCodes.INVALID_DATA;
    }

    /* unsigned integer - 32 bits */
    int writeUInt32ob(long v)
    {
        if (v < 0)
            return CodecReturnCodes.INVALID_DATA;

        if (v < 0xFEL)
        {
            if (hasRemaining(1))
            {
                writeByte((int)v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFL)
        {
            if (hasRemaining(3))
            {
                writeByte(0xFE);
                writeUShort((short)v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFL)
        {
            if (hasRemaining(5))
            {
                writeByte(0xFF);
                writeUInt((int)v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
            return CodecReturnCodes.INVALID_DATA;
    }

    /* Unsigned Long - 64 bits */
    int writeULong64ls(long v)
    {
        if (v < 0)
        {
            if (hasRemaining(8))
            {
                writeLong(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFL)
        {
            if (hasRemaining(1))
            {
                writeByte((byte)(v & 0xFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFL)
        {
            if (hasRemaining(2))
            {
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFL)
        {
            if (hasRemaining(3))
            {
                writeByte((byte)((v >> 16) & 0xFF));
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFL)
        {
            if (hasRemaining(4))
            {
                writeUInt(v & 0xFFFFFFFF);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFFFL)
        {
            if (hasRemaining(5))
            {
                writeByte((byte)((v >> 32) & 0xFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFFFFFL)
        {
            if (hasRemaining(6))
            {
                writeUShort((short)((v >> 32) & 0xFFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFFFFFFFL)
        {
            if (hasRemaining(7))
            {
                writeByte((byte)((v >> 48) & 0xFF));
                writeUShort((short)((v >> 32) & 0x00FFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
        {
            if (hasRemaining(8))
            {
                writeLong(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
    }

    int writeULong64lsWithLength(long v)
    {
        if (v < 0)
        {
            if (hasRemaining(9))
            {
                writeByte(8);
                writeLong(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFL)
        {
            if (hasRemaining(2))
            {
                writeByte(1);
                writeByte((byte)(v & 0xFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFL)
        {
            if (hasRemaining(3))
            {
                writeByte(2);
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFL)
        {
            if (hasRemaining(4))
            {
                writeByte(3);
                writeByte((byte)((v >> 16) & 0xFF));
                writeUShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFL)
        {
            if (hasRemaining(5))
            {
                writeByte(4);
                writeUInt(v & 0xFFFFFFFF);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFFFL)
        {
            if (hasRemaining(6))
            {
                writeByte(5);
                writeByte((byte)((v >> 32) & 0xFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFFFFFL)
        {
            if (hasRemaining(7))
            {
                writeByte(6);
                writeUShort((short)((v >> 32) & 0xFFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if (v <= 0xFFFFFFFFFFFFFFL)
        {
            if (hasRemaining(8))
            {
                writeByte(7);
                writeByte((byte)((v >> 48) & 0xFF));
                writeUShort((short)((v >> 32) & 0x00FFFF));
                writeUInt(v & 0xFFFFFFFFL);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else
        {
            if (hasRemaining(9))
            {
                writeByte(8);
                writeLong(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
    }
    
    /*
     * This method combined with the calling code replaces the previous writeUShort15rb method.
     * If the value v is smaller than 0x80 the value is written on the wire as one byte, otherwise it is two bytes.
     * The calling code must check the value and ensure that the buffer is sufficient enough to write this value (either one or two bytes).
     * If the value is smaller than 0x80 the calling code uses writeByte method.
     * This method is called only when the v >= 0x80.
     */
    void writeUShort15rbLong(short v)
    {
        // the v is verified in the preceding code to be >=0
        short v1 = (short)(v | 0x8000);
        writeShort(v1);
    }

    /*
     * This method combined with the calling code replaces the previous writeUShort16ob method.
     * If the value v is smaller than 0xFE the value is written on the wire as one byte, otherwise it is three bytes.
     * The calling code must check the value and ensure that the buffer is sufficient enough
     * to write this value (either one or three bytes).
     * If the value is smaller than 0xFE the calling code uses writeByte method.
     * This method is called only when the v >= 0xFE.
     */
    void writeUShort16obLong(int v)
    {
        // the v is verified in the preceding code to be >= 0
        writeByte(0xFE);
        writeUShort((short)v);
    }

    void write(BufferImpl buf)
    {
        if (buf.length() > 0)
        {
            String bufAsString = buf.dataString();
            if (bufAsString != null)
            {
                // copy ASCII data (8bits).
                int pos = buf.position();
                for (int idx = 0; idx < buf.length(); idx++)
                {
                    _buffer.put((byte)(0xff & bufAsString.charAt(idx + pos)));
                }
            }
            else
            {
                ByteBuffer bb = buf.data();
                assert (bb != null) : "Buffer has no backing data set.";
                int saveLimit = bb.limit();
                int savePos = bb.position();
                bb.position(buf.position());
                bb.limit(buf.position() + buf.length());
                _buffer.put(bb);
                bb.limit(saveLimit);
                bb.position(savePos);
            }
        }
    }
    
    int writeBytes(String s, String charset)
    {
        try
        {
            _buffer.put(s.getBytes(charset));
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INVALID_DATA;
        }
        return CodecReturnCodes.SUCCESS;
    }

    void writeUByte(int v)
    {
        assert (v >= 0) : "the value should not be negative";
        _buffer.put((byte)v);
    }

    void writeUShort(int v)
    {
        _buffer.putShort((short)v);
    }

    void writeUInt(long v)
    {
        _buffer.putInt((int)v);
    }

    void writeBoolean(boolean v)
    {
        if (v)
            writeByte(1);
        else
            writeByte(0);
    }

    void writeByte(int v)
    {
        _buffer.put((byte)v);
    }

    void writeChar(int v)
    {
        _buffer.putChar((char)v);
    }

    void writeDouble(double v)
    {
        _buffer.putDouble(v);
    }

    void writeFloat(float v)
    {
        _buffer.putFloat(v);
    }

    void writeInt(int v)
    {
        _buffer.putInt(v);
    }

    void writeLong(long v)
    {
        _buffer.putLong(v);
    }

    void writeShort(int v)
    {
        _buffer.putShort((short)v);
    }

    // this is reading method, added for utilities
    @Override
    short getUShort15rb()
    {
        short val = _buffer.get();
        if (val < 0)
            val &= 0xFF;

        if ((val & 0x80) != 0)
        {
            short b = _buffer.get();
            if (b < 0)
                b &= 0xFF;
            return (short)(((val & 0x7F) << 8) + b);
        }

        return val;
    }

}
