/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Int;

class IntImpl implements Int
{
    long _longValue;
    boolean _isBlank;
    
    // for value(String) method
    private String trimmedVal;

    IntImpl()
    {
    }
    
    @Override
    public void clear()
    {
        _longValue = 0;
    }
    
    void blank()
    {
        clear();
        _isBlank = true;
    }
	
    public int copy(Int destInt)
    {
        if (null == destInt)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((IntImpl)destInt)._longValue = _longValue;
        ((IntImpl)destInt)._isBlank = _isBlank;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
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
    public long toLong()
    {
        return _longValue;
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.PrimitiveEncoder.encodeInt((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }
    
    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeInt(iter, this);
    }

    @Override
    public String toString()
    {
        return Long.toString(_longValue);
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
            value(Long.parseLong(trimmedVal));
            return CodecReturnCodes.SUCCESS;
        }
        catch (NumberFormatException e)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public boolean equals(Int thatInt)
    {
        return ((thatInt != null) &&
                (_longValue == thatInt.toLong()) &&
                (_isBlank == thatInt.isBlank()));
    }
}
