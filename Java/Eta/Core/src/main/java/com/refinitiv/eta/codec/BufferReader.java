/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;
import java.lang.Float;
import java.lang.Double;

/* All of the reads are absolute.
 * The internal buffer's position is not changed.
 * This reader keeps track of the position.
 */
abstract class BufferReader
{
    ByteBuffer _buffer;
    int _position;
    
    void data(ByteBuffer buffer)
    {
        _buffer = buffer;
    }
	
    ByteBuffer buffer()
    {
        return _buffer;
    }

    int position()
    {
        return _position;
    }

    void position(int pos) throws Exception
    {
        _position = pos;
        if (pos > _buffer.limit())
            readByte(); // force exception
    }
    
    int readShort() throws Exception
    {
        short s = _buffer.getShort(_position);
        _position += 2; // short
        return s;
    }

    byte readByte() throws Exception
    {
        byte b = _buffer.get(_position);
        _position += 1; // byte
        return b;
    }

    int readInt() throws Exception
    {
        int i = _buffer.getInt(_position);
        _position += 4; // int
        return i;
    }

    long readLong() throws Exception
    {
        long val = _buffer.getLong(_position);
        _position += 8; // long
        return val;
    }

    double readDouble() throws Exception
    {
        return Double.longBitsToDouble(readLong());
    }

    float readFloat() throws Exception
    {
        return Float.intBitsToFloat(readInt());
    }

    boolean readBoolean() throws Exception
    {
        int val = readUnsignedByte();
        return val != 0;
    }

    int readUnsignedByte() throws Exception
    {
        short val = readByte();
        if (val < 0)
            val &= 0xFF;
        return val;
    }

    int readUnsignedShort() throws Exception
    {
        int val = readShort();

        if (val < 0)
            val &= 0xFFFF;

        return val;
    }

    /* Reads an unsigned 32 bit number. */
    long readUnsignedInt() throws Exception
    {
        long val = readInt();
        if (val < 0)
            val &= 0xFFFFFFFFL;
        return val;
    }

    /* Java cannot represent an unsigned long larger than 9,223,372,036,854,775,807 (0x7FFFFFFFFFFFFFFF) */
    /* Returns unsigned long */
    long readULong() throws Exception
    {
        long val = readLong();
        return val;
    }
    
    void skipBytes(int i) throws Exception
    {
        _position += i;
        if (_position > _buffer.limit())
            _buffer.get(_position); // force exception
    }

    /* Relative read of an unsigned 32 bit number. */
    long readRelativeUnsignedInt()
    {
        long val = _buffer.getInt();
        if (val < 0)
            val &= 0xFFFFFFFFL;
        return val;
    }

    // The following methods are implemented in the subclasses according to the format the transport expects
    abstract short readUShort15rb() throws Exception;
    abstract int readUShort16ob() throws Exception;
    abstract int readInt16ls(int size) throws Exception;
    abstract int readInt32ls(int size) throws Exception;
    abstract int readUInt16ls(int size) throws Exception;
    abstract int readUInt30rb() throws Exception;
    abstract long readUInt32ob() throws Exception;
    abstract long readUInt32ls(int size) throws Exception;
    abstract long readLong64ls(int size) throws Exception;
    abstract long readULong64ls(int size) throws Exception;
    abstract byte majorVersion();
    abstract byte minorVersion();
    abstract void clear();
    abstract short readRelativeUShort15rb() throws Exception;
    abstract int readRelativeUShort16ob() throws Exception;
}
