/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.math.BigInteger;

import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.UInt;

class UIntImpl implements UInt
{
    long _longValue;
    long _helper;
    byte[] _bytes = new byte[9];
    boolean _isBlank;

    // for value(String) method
    private String trimmedVal;

    UIntImpl()
    {
    }

    @Override
    public void clear()
    {
        _longValue = 0;
    }

    public int copy(UInt destUInt)
    {
        if (null == destUInt)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((UIntImpl)destUInt)._longValue = _longValue;
        ((UIntImpl)destUInt)._isBlank = _isBlank;

        return CodecReturnCodes.SUCCESS;
    }

    void blank()
    {
        clear();
        _isBlank = true;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }

    @Override
    public long toLong()
    {
        return _longValue;
    }

    @Override
    public BigInteger toBigInteger()
    {
        _helper = _longValue;
        for (int i = 8; i > 0; i--)
        {
            _bytes[i] = (byte)(_helper);
            _helper = (_helper >> 8);
        }
        _bytes[0] = (byte)0;

        return new BigInteger(_bytes);
    }

    @Override
    public void value(int value)
    {
        _longValue = value;
        _isBlank = false;
    }

    @Override
    public void value(long value)
    {
        _longValue = value;
        _isBlank = false;
    }

    @Override
    public int value(BigInteger value)
    {
        if (value == null)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _longValue = value.longValue();
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.PrimitiveEncoder.encodeUInt((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeUInt(iter, this);
    }

    @Override
    public String toString()
    {
        if (_longValue >= 0)
        {
            return Long.toString(_longValue);
        }
        else
        {
            return toBigInteger().toString();
        }
    }

    @Override
    public int value(String value)
    {
        if (value == null)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        trimmedVal = value.trim();
        if (trimmedVal.length() == 0)
        {
            // blank
            blank();
            return CodecReturnCodes.SUCCESS;
        }

        try
        {
            value(Long.parseLong(value));
            return CodecReturnCodes.SUCCESS;
        }
        catch (NumberFormatException e)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public boolean equals(UInt thatUInt)
    {
        return ((thatUInt != null) &&
                (_longValue == thatUInt.toLong()) &&
                (_isBlank == thatUInt.isBlank()));
    }
}
