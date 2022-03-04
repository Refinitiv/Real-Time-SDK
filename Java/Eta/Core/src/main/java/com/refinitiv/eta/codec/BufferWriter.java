/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;

abstract class BufferWriter
{
    ByteBuffer _buffer;
	
    // Bytes reserved for data that will need to be written later
    // (e.g. count for a container's entries after summary data is encoded).
    int _reservedBytes;

    ByteBuffer buffer()
    {
        return _buffer;
    }
	
    int position()
    {
        return _buffer.position();
    }

    void position(int pos)
    {
        _buffer.position(pos);
    }
	
    void reserveBytes(int bytes)
    {
        _reservedBytes += bytes;
    }

    void unreserveBytes(int bytes)
    {
        _reservedBytes -= bytes;
    }
	
    boolean hasRemaining(int v)
    {
        return _buffer.remaining() - _reservedBytes >= v;
    }
	
    void skipBytes(int v)
    {
        _buffer.position(_buffer.position() + v);
    }
	
    abstract void write(BufferImpl buf);
    abstract void writeBoolean(boolean v);
    abstract void writeByte(int v);
    abstract void writeUByte(int v);
    abstract void writeShort(int v);
    abstract void writeUShort(int v);
    abstract void writeChar(int v);
    abstract void writeInt(int v);
    abstract void writeUInt(long v);
    abstract void writeLong(long v);
    abstract void writeFloat(float v);
    abstract void writeDouble(double v);
    abstract int writeBytes(String s, String charset);
    abstract int writeUInt32ob(long v);
    abstract void writeUShort16obLong(int v);
    abstract void writeUShort15rbLong(short v);
    abstract int writeUInt30rb(int v);
    abstract int writeUInt16ls(int v);
    abstract int writeUInt16lsWithLength(int v);
    abstract int writeLong64ls(long v);
    abstract int writeLong64lsWithLength(long v);
    abstract int writeLong64lsBy2WithLength(long v);
    abstract int writeULong64ls(long v);
    abstract int writeULong64lsWithLength(long v);
    abstract byte majorVersion();
    abstract byte minorVersion();
    abstract void clear();

    // This is a reading method, added for utilities.
    // The method returns UShort15rb read at the buffer position.
    abstract short getUShort15rb();
	
}
