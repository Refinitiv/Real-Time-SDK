/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;

class RmtesBufferImpl implements RmtesBuffer
{
    private int _length; /* length of data used in the buffer. */
    private int _allocatedLength; /* Total allocated length of the buffer. */
    private ByteBuffer _data; /* The ByteBuffer of data */

    RmtesBufferImpl(int dataLength, ByteBuffer byteData, int allocLength)
    {
        _length = dataLength;
        _data = byteData;
        _allocatedLength = allocLength;
    }

    RmtesBufferImpl(int x)
    {
        _data = ByteBuffer.allocate(x);
        _allocatedLength = x;
        _length = 0;
    }

    @Override
    public String toString()
    {
        String output = "";
        output = new String(_data.array(), Charset.forName("UTF-16"));

        return output;
    }

    @Override
    public int length()
    {
        return _length;
    }

    @Override
    public int allocatedLength()
    {
        return _allocatedLength;
    }

    @Override
    public ByteBuffer byteData()
    {
        return _data;
    }

    @Override
    public void length(int x)
    {
        _length = x;
    }

    @Override
    public void allocatedLength(int x)
    {
        _allocatedLength = x;
    }

    @Override
    public void data(ByteBuffer x)
    {
        _data = x;
    }

    @Override
    public void clear()
    {
        _data.clear();
        _length = 0;
        _allocatedLength = _data.limit();
    }

    /* Deprecated to leverage ByteBuffer instead of CharBuffer */
    @Deprecated
    @Override
    public void data(CharBuffer x)
    {
        x.position(0);
        for (; x.position() < x.limit();)
        {
            char next = x.get();
            _data.put((byte)((next & 0xFF00) >> 8));
            _data.put((byte)(next & 0x00FF));
        }
    }

    /* Deprecated to leverage ByteBuffer instead of CharBuffer */
    @Deprecated
    @Override
    public CharBuffer data()
    {
        return _data.asCharBuffer();
    }
}
